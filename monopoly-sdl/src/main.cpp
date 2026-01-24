// src/main.cpp
#include <SDL2/SDL.h>
#include <vector>
#include <iostream>
#include "Board.h"
#include "MapLoader.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* args[]) {
    // 1. 初始化
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Pure SDL2 Monopoly Engine", 
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 2. 載入資料
    std::cout << "Loading map..." << std::endl;
    std::vector<Tile> mapTiles = MapLoader::LoadMap("assets/map_data.txt");
    
    if (mapTiles.empty()) {
        std::cerr << "Failed to load map data!" << std::endl;
        // 這裡暫時不退出，方便測試環境
    } else {
        std::cout << "Map loaded. Total tiles: " << mapTiles.size() << std::endl;
    }

    // 3. 遊戲迴圈
    bool quit = false;
    SDL_Event e;

    while (!quit) {
        // Input
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 黑底
        SDL_RenderClear(renderer);

        // 繪製地圖格子 (Debug View)
        for (const auto& tile : mapTiles) {
            SDL_Rect tileRect = { tile.x, tile.y, 40, 40 };
            
            // 根據類型給不同顏色
            if (tile.type == TILE_START) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);      // 綠
            else if (tile.type == TILE_PROPERTY) SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // 白
            else if (tile.type == TILE_JAIL) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);    // 紅
            else SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // 藍 (其他)

            SDL_RenderDrawRect(renderer, &tileRect); // 畫空心框
        }

        SDL_RenderPresent(renderer);
    }

    // 4. 清理
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
