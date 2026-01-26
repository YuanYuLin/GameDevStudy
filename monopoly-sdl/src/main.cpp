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

// 遊戲狀態
enum GameState {
    STATE_WAIT_ROLL,
    STATE_MOVING,
    STATE_WAIT_DECISION,
    STATE_TURN_END // 新增：回合結算過渡
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// 輔助函數：根據 ID 查找玩家指標
Player* GetPlayerById(std::vector<Player>& players, int id) {
    for (auto& p : players) {
        if (p.id == id) return &p;
    }
    return nullptr;
}

int main(int argc, char* args[]) {
    std::srand(std::time(nullptr));
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return -1;

    SDL_Window* window = SDL_CreateWindow("Monopoly Multiplayer", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 1. 載入資源
    std::vector<Tile> mapTiles = MapLoader::LoadMap("assets/map_data.txt");
    TextRenderer textEngine(renderer, "assets/font.bmp", 8, 8);

    // 2. 初始化玩家 (P1 紅色, P2 藍色)
    std::vector<Player> players;
    players.emplace_back(1, 0, mapTiles, 255, 50, 50); // ID 1
    players.emplace_back(2, 0, mapTiles, 50, 50, 255); // ID 2

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
                            // 切換到下一個玩家
                            currentPlayerIdx = (currentPlayerIdx + 1) % players.size();
                            messageLog = "P" + std::to_string(players[currentPlayerIdx].id) + "'S TURN";
                            currentState = STATE_WAIT_ROLL;
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

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
