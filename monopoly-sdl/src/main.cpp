#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <string>
#include "Board.h"
#include "MapLoader.h"
#include "Player.h"
#include "TextRenderer.h"
#include "CardSystem.h"
#include "Config.h"

// 遊戲狀態
enum GameState {
    STATE_WAIT_ROLL,
    STATE_MOVING,
    STATE_WAIT_DECISION,
    STATE_SHOW_CARD, 
    STATE_TURN_END,
    STATE_GAME_OVER
};

void HandleBankruptcy(Player& player, std::vector<Tile>& map) {
    std::cout << "Player " << player.id << " is BANKRUPT!" << std::endl;
    player.isBankrupt = true;
    player.money = 0; // 歸零顯示

    // 資產清算：把他的地全部變回無主地
    // (進階規則是可以交給債主，但這裡我們先做簡單的充公)
    for (auto& tile : map) {
        if (tile.ownerId == player.id) {
            tile.ownerId = -1; 
        }
    }
}

int CheckWinner(const std::vector<Player>& players) {
    int aliveCount = 0;
    int winnerId = -1;
    
    for (const auto& p : players) {
        if (!p.isBankrupt) {
            aliveCount++;
            winnerId = p.id;
        }
    }

    if (aliveCount <= 1) return winnerId; // 只剩一人 (或更少)，遊戲結束
    return -1; // 遊戲繼續
}

// 輔助函數：根據 ID 查找玩家指標
Player* GetPlayerById(std::vector<Player>& players, int id) {
    for (auto& p : players) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

struct Color { Uint8 r, g, b; };
const Color PLAYER_COLORS[] = {
    {255, 50, 50},   // P1 紅
    {50, 50, 255},   // P2 藍
    {50, 200, 50},   // P3 綠
    {255, 255, 50},  // P4 黃
    {255, 128, 0},   // P5 橘
    {200, 50, 200}   // P6 紫
};

int main(int argc, char* args[]) {
    std::srand(std::time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

    // 必須在建立視窗之前讀取，因為需要寬高資訊
    GameConfig config = ConfigLoader::LoadConfig("assets/settings.txt");
    std::cout << "Config Loaded: " << config.windowWidth << "x" << config.windowHeight 
              << ", Players: " << config.playerCount << std::endl;

    SDL_Window* window = SDL_CreateWindow("Monopoly Multiplayer", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        config.windowWidth, config.windowHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // --- 載入卡片系統 ---
    CardSystem cardDeck;
    if (!cardDeck.Load("assets/cards.txt")) {
        std::cerr << "Failed to load cards!" << std::endl;
        // 這裡可以做錯誤處理，或塞幾張預設卡
    }
    // 暫存當前抽到的卡片，用於顯示
    const Card* currentCard = nullptr;

    // 1. 載入資源
    std::vector<Tile> mapTiles = MapLoader::Load("assets/map_data.txt");
    TextRenderer textEngine(renderer, "assets/font.bmp", 8, 8);

    // 2. 初始化玩家 (P1 紅色, P2 藍色)
    std::vector<Player> players;
    for (int i = 0; i < config.playerCount; i++) {
        // 確保顏色不會超出陣列範圍
        Color c = PLAYER_COLORS[i % 6]; 
        
        // 建立玩家 (ID 從 1 開始)
        // 注意：這裡我們需要修改 Player 建構子來支援自訂初始金額
        // 如果你的 Player 建構子還沒改，可以先傳入預設值
        players.emplace_back(i + 1, 0, mapTiles, c.r, c.g, c.b);
        
        // 設定初始金額 (需要去 Player.h 把 money 設為 public 或者提供 SetMoney 方法)
        players.back().money = config.startMoney; 
    }

    // 3. 回合控制變數
    int currentPlayerIdx = 0; // 0 代表 P1, 1 代表 P2
    GameState currentState = STATE_WAIT_ROLL;
    std::string messageLog = "P1 START! PRESS SPACE.";
    int lastDice = 0;
    
    // 時間變數
    Uint32 lastTime = SDL_GetTicks();
    float deltaTime = 0.0f;
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        Uint32 currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // 取得當前玩家的參照 (Reference)
        Player& currentPlayer = players[currentPlayerIdx];

        // --- Input ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_KEYDOWN) {
                switch (currentState) {
                    case STATE_WAIT_ROLL:
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            lastDice = (std::rand() % 6) + 1;
                            currentPlayer.RollDice(lastDice);
                            messageLog = "P" + std::to_string(currentPlayer.id) + " ROLLED " + std::to_string(lastDice);
                            currentState = STATE_MOVING;
                        }
                        break;
                    
                    case STATE_WAIT_DECISION:
                        if (e.key.keysym.sym == SDLK_y) {
                            // 買地
                            Tile& t = mapTiles[currentPlayer.currentTileIndex];
                            if (currentPlayer.money >= t.price) {
                                currentPlayer.Pay(t.price);
                                t.ownerId = currentPlayer.id;
                                messageLog = "P" + std::to_string(currentPlayer.id) + " BOUGHT LAND!";
                            } else {
                                messageLog = "NOT ENOUGH CASH!";
                            }
                            currentState = STATE_TURN_END;
                        }
                        else if (e.key.keysym.sym == SDLK_n) {
                            messageLog = "PASSED.";
                            currentState = STATE_TURN_END;
                        }
                        break;
                    
                    case STATE_TURN_END:
                        // 按任意鍵換下一位
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            // --- 檢查是否有破產發生 ---
                            if (currentPlayer.money < 0) {
                                HandleBankruptcy(currentPlayer, mapTiles);
                                messageLog = "P" + std::to_string(currentPlayer.id) + " WENT BANKRUPT!";
                            }

                            // --- 檢查是否遊戲結束 ---
                            int winnerId = CheckWinner(players);
                            if (winnerId != -1) {
                                currentState = STATE_GAME_OVER;
                                messageLog = "GAME OVER! WINNER: P" + std::to_string(winnerId);
                                break; // 跳出 switch
                            }

                            // --- 切換到下一位倖存者 ---
                            // 使用 do-while 迴圈，跳過所有已破產的玩家
                            int nextIdx = currentPlayerIdx;
                            do {
                                nextIdx = (nextIdx + 1) % players.size();
                            } while (players[nextIdx].isBankrupt);

                            currentPlayerIdx = nextIdx;
                            
                            // 更新狀態文字
                            messageLog = "P" + std::to_string(players[currentPlayerIdx].id) + "'S TURN";
                            currentState = STATE_WAIT_ROLL;
                        }
                        break;
                    case STATE_SHOW_CARD:
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            currentState = STATE_TURN_END;
                            messageLog = "TURN END.";
                        }
                        break;
                    case STATE_GAME_OVER:
                        // 遊戲結束後按 ESC 退出
                        if (e.key.keysym.sym == SDLK_ESCAPE) {
                            quit = true;
                        }
                        break;
                    default: break;
                }
            }
        }

        // --- Update ---
        // 更新所有玩家的位置 (如果有動畫的話)
        for (auto& p : players) {
            p.Update(deltaTime, mapTiles);
        }

        // 檢查當前玩家移動是否結束
        if (currentState == STATE_MOVING && !currentPlayer.isMoving) {
            // 移動結束，觸發事件
            Tile& tile = mapTiles[currentPlayer.currentTileIndex];
            
            if (tile.type == TILE_PROPERTY) {
                if (tile.ownerId == -1) {
                    // 無主地 -> 詢問購買
                    messageLog = "BUY FOR $" + std::to_string(tile.price) + "? (Y/N)";
                    currentState = STATE_WAIT_DECISION;
                } 
                else if (tile.ownerId == currentPlayer.id) {
                    // 自己的地
                    messageLog = "OWN PROPERTY. SAFE.";
                    currentState = STATE_TURN_END;
                }
                else {
                    // **支付過路費邏輯**
                    Player* owner = GetPlayerById(players, tile.ownerId);
                    if (owner) {
                        messageLog = "PAID RENT $" + std::to_string(tile.rent) + " TO P" + std::to_string(owner->id);
                        currentPlayer.Pay(tile.rent);
                        owner->Receive(tile.rent);
                    }
                    currentState = STATE_TURN_END;
                }
            } else if (tile.type == TILE_CHANCE) {
                // === 觸發機會卡 ===
                currentCard = cardDeck.DrawRandomCard();
                
                if (currentCard) {
                    messageLog = "CHANCE: " + currentCard->description;
                    
                    // 執行卡片效果
                    if (currentCard->type == EFFECT_MONEY) {
                        if (currentCard->value > 0) currentPlayer.Receive(currentCard->value);
                        else currentPlayer.Pay(-currentCard->value); // Pay 函數只接受正數，所以要轉號
                    }
                    else if (currentCard->type == EFFECT_MOVE) {
                        currentPlayer.Teleport(currentCard->value, mapTiles);
                        // 這裡有個細節：傳送後要不要觸發新格子的事件？
                        // 為了簡單起見，傳送後直接結束回合
                    }
                    
                    currentState = STATE_SHOW_CARD; // 進入展示狀態
                } else {
                    messageLog = "NO CARDS LOADED!";
                    currentState = STATE_TURN_END;
                }
            } else {
                // 其他格子直接結束回合
                messageLog = "LANDED ON " + tile.name;
                currentState = STATE_TURN_END;
            }
        }

        // --- Render ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 1. 畫地圖
        for (const auto& tile : mapTiles) {
	    SDL_Rect tileRect = { tile.x, tile.y, tile.w, tile.h };
            
            // 根據擁有者變色
            if (tile.ownerId == 1) SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255); // P1 紅
            else if (tile.ownerId == 2) SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255); // P2 藍
            else if (tile.type == TILE_PROPERTY) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            else SDL_SetRenderDrawColor(renderer, 50, 150, 50, 255); // 其他
            
            SDL_RenderDrawRect(renderer, &tileRect);
        }

        // 2. 畫所有玩家
        for (auto& p : players) {
            p.Render(renderer);
        }

        // 3. UI 資訊
        // 標題與當前狀態
        textEngine.DrawText(renderer, 50, 100, "MULTIPLAYER MODE", 2.0f);
        textEngine.DrawText(renderer, 50, 140, messageLog, 1.5f);

        // 顯示 P1 資訊
        std::string p1Text = "P1: $" + std::to_string(players[0].money);
        if (currentPlayerIdx == 0) p1Text = "> " + p1Text; // 當前回合箭頭指示
        textEngine.DrawText(renderer, 50, 400, p1Text, 1.5f);

        // 顯示 P2 資訊
        std::string p2Text = "P2: $" + std::to_string(players[1].money);
        if (currentPlayerIdx == 1) p2Text = "> " + p2Text;
        textEngine.DrawText(renderer, 50, 430, p2Text, 1.5f);

        // 提示
        if (currentState == STATE_TURN_END) {
             textEngine.DrawText(renderer, 300, 300, "PRESS SPACE NEXT", 1.5f);
        }

        if (currentState == STATE_SHOW_CARD && currentCard) {
            // 畫一個大白框當作卡片背景
            SDL_Rect cardRect = { 200, 150, 400, 200 };
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderFillRect(renderer, &cardRect);
            
            // 畫黑框邊線
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderDrawRect(renderer, &cardRect);

            // 顯示卡片內容
            textEngine.DrawText(renderer, 220, 180, "CHANCE CARD!", 2.0f);
            
            // 為了讓描述文字清楚，我們可能需要手動換行，這裡先簡單顯示一行
            // 注意：我們把底線替換回空白鍵顯示，看起來比較自然
            std::string displayDesc = currentCard->description;
            // 簡單的替換字元邏輯 (Optional)
            for(auto& c : displayDesc) if(c == '_') c = ' ';

            textEngine.DrawText(renderer, 220, 230, displayDesc, 1.5f);
            
            if (currentCard->type == EFFECT_MONEY) {
                std::string valStr = (currentCard->value > 0 ? "+$" : "-$") + std::to_string(std::abs(currentCard->value));
                textEngine.DrawText(renderer, 220, 260, valStr, 2.0f);
            }
            
            textEngine.DrawText(renderer, 220, 310, "PRESS SPACE...", 1.0f);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
