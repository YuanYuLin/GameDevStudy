#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
#include <ctime>   // for time()
#include <cstdlib> // for rand(), srand()
#include "Board.h"
#include "MapLoader.h"
#include "Player.h" // 引入新寫的 Player

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

    // --- 時間變數 ---
    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime = 0;
    float deltaTime = 0.0f;

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // --- 1. 計算 Delta Time (秒) ---
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // --- 2. 輸入處理 ---
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_SPACE) {
                    // 按空白鍵擲骰子 (1~6)
                    if (!player1.isMoving) {
                        int dice = (std::rand() % 6) + 1;
                        player1.RollDice(dice);
                    }
                }
            }
        }

        // --- 3. 更新邏輯 (Update) ---
        player1.Update(deltaTime, mapTiles);

        // --- 4. 渲染 (Render) ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // A. 畫地圖
        for (const auto& tile : mapTiles) {
            SDL_Rect tileRect = { tile.x, tile.y, tile.w, tile.h };
            if (tile.type == TILE_START) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            else if (tile.type == TILE_PROPERTY) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            else SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &tileRect);
        }

        // B. 畫玩家
        player1.Render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
