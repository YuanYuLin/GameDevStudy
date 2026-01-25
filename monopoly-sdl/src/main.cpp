#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
#include <ctime>   // for time()
#include <cstdlib> // for rand(), srand()
#include "Board.h"
#include "MapLoader.h"
#include "Player.h" // 引入新寫的 Player
#include "TextRenderer.h" 

// 定義遊戲狀態
enum GameState {
    STATE_WAIT_ROLL,
    STATE_MOVING,
    STATE_WAIT_DECISION
};

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* args[]) {
    // 初始化亂數種子
    std::srand(std::time(nullptr));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Monopoly Engine - Movement Test", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // --- 載入地圖 ---
    std::vector<Tile> mapTiles = MapLoader::LoadMap("assets/map_data.txt");
    if (mapTiles.empty()) return -1;

    // --- 建立玩家 ---
    // 玩家 ID 1, 從第 0 格開始
    Player player1(1, 0, mapTiles);

    // --- 初始化字體系統 ---
    // 假設你下載的 font.bmp 每個字是 8x8 像素
    TextRenderer textEngine(renderer, "assets/font.bmp", 8, 8);

    // --- 時間變數 ---
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = 0;
    float deltaTime = 0.0f;

    bool quit = false;
    SDL_Event e;
    GameState currentState = STATE_WAIT_ROLL;
    std::string messageLog = "GAME START! PRESS SPACE.";
    int lastDice = 0; // 記錄上次骰出的數字

    while (!quit) {
        // --- 1. 計算 Delta Time (秒) ---
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // --- 2. 輸入處理 ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_KEYDOWN) {
         // 根據不同狀態，按鍵有不同反應
                switch (currentState) {
                    case STATE_WAIT_ROLL:
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            lastDice = (std::rand() % 6) + 1;
                            player1.RollDice(lastDice);
                            // messageLog = "ROLLING...";
                            messageLog = "DICE: " + std::to_string(lastDice);
                            currentState = STATE_MOVING; // 切換狀態
                        }
                        break;
                    
                    case STATE_WAIT_DECISION:
                        if (e.key.keysym.sym == SDLK_y) {
                            // 購買邏輯
                            Tile& currentTile = mapTiles[player1.currentTileIndex];
                            if (player1.money >= currentTile.price) {
                                player1.Pay(currentTile.price);
                                currentTile.ownerId = player1.id;
                                messageLog = "BOUGHT " + currentTile.name + "!";
                            } else {
                                messageLog = "NOT ENOUGH MONEY!";
                            }
                            currentState = STATE_WAIT_ROLL; // 回合結束，回到等待
                        }
                        else if (e.key.keysym.sym == SDLK_n) {
                            messageLog = "YOU PASSED.";
                            currentState = STATE_WAIT_ROLL; // 回合結束
                        }
                        break;

                    case STATE_MOVING:
                        // 移動中不接受任何輸入
                        break;
                }
            }
        }
        // --- 2. 邏輯更新 (Update) ---
        if (currentState == STATE_MOVING) {
            player1.Update(deltaTime, mapTiles);

            // 檢查是否移動結束
            if (!player1.isMoving) {
                // 玩家剛停下來，觸發事件！
                Tile& tile = mapTiles[player1.currentTileIndex];
                
                if (tile.type == TILE_PROPERTY) {
                    if (tile.ownerId == -1) {
                        // 無主地 -> 問玩家要不要買
                        messageLog = "BUY " + tile.name + " FOR $" + std::to_string(tile.price) + "? (Y/N)";
                        currentState = STATE_WAIT_DECISION;
                    } 
                    else if (tile.ownerId == player1.id) {
                        // 自己的地 -> 沒事發生
                        messageLog = "WELCOME HOME!";
                        currentState = STATE_WAIT_ROLL;
                    }
                    else {
                        // 別人的地 -> 付錢
                        messageLog = "PAID RENT $" + std::to_string(tile.rent);
                        player1.Pay(tile.rent);
                        // TODO: 這裡應該要加錢給地主
                        currentState = STATE_WAIT_ROLL;
                    }
                } else {
                    // 非地產格 (起點、機會...)
                    messageLog = "LANDED ON " + tile.name;
                    currentState = STATE_WAIT_ROLL;
                }
            }
        }

        // --- 3. 渲染 (Render) ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // A. 畫地圖
        for (const auto& tile : mapTiles) {
            SDL_Rect tileRect = { tile.x, tile.y, tile.w, tile.h };
            if (tile.type == TILE_START) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            else if (tile.type == TILE_PROPERTY) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            else SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &tileRect);
	    //textEngine.DrawText(renderer, tile.x, tile.y, std::to_string(tile.id));
        }

        // B. 畫玩家
        player1.Render(renderer);

        // 顯示操作提示
        textEngine.DrawText(renderer, 50, 100, "MONOPOLY ENGINE - LINUX", 3.0f); // 2倍放大
        textEngine.DrawText(renderer, 50, 140, "PRESS SPACE TO ROLL", 2.0f);

        textEngine.DrawText(renderer, 50, 240, messageLog, 1.5f); // 顯示當前訊息
        textEngine.DrawText(renderer, 50, 400, "P1 MONEY: $" + std::to_string(player1.money), 2.5f);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
