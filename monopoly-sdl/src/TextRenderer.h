#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <SDL2/SDL.h>
#include <string>
#include <iostream>
#include <vector>

class TextRenderer {
private:
    SDL_Texture* fontTexture;
    int charWidth;
    int charHeight;
    int cols; // 圖片一行有幾個字 (通常是 16)
    int out_count;
    std::vector<int> charMap;

public:
    // 建構子：載入字體圖片
    TextRenderer(SDL_Renderer* renderer, const std::string& filename, int w, int h) 
        : fontTexture(nullptr), charWidth(w), charHeight(h), cols(16), out_count(0), charMap(256) {
        
        // Initialize charMap with identity mapping
        for(int i = 0; i < 256; ++i) {
            charMap[i] = i;
        }
        // Custom mappings
        charMap[' '] = 0;
        charMap['$'] = 4;
        charMap['-'] = 13;
        charMap['.'] = 14;
        charMap[':'] = 26;
        for(int i = 0; i <= 9; ++i) {
            charMap['0' + i] = 16 + i;
        }
        
        SDL_Surface* surface = SDL_LoadBMP(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load font: " << filename << std::endl;
            // 這裡失敗沒關係，我們後面會有防呆
            return;
        }

        // 設定黑色 (0,0,0) 為透明色 (根據你的圖片調整)
        SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 0, 0, 0));

        fontTexture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }

    ~TextRenderer() {
        if (fontTexture) {
            SDL_DestroyTexture(fontTexture);
        }
    }

    // 核心函數：畫字串
    // x, y: 螢幕座標
    // text: 要顯示的文字
    // scale: 放大倍率 (1.0 = 原大, 2.0 = 兩倍大)
    void DrawText(SDL_Renderer* renderer, int x, int y, std::string text, float scale = 1.0f, bool debug = false) {
        if (!fontTexture) return; // 沒圖就不畫

        int cursorX = x;
        int cursorY = y;
        
        // 設定要畫多大
        int displayW = (int)(charWidth * scale);
        int displayH = (int)(charHeight * scale);

        for (char c : text) {
            // 處理換行
            if (c == '\n') {
                cursorX = x;
                cursorY += displayH;
                continue;
            }

            // --- 數學魔法：計算字元在圖片上的位置 ---
            // ASCII 碼轉為整數索引 (0~255)
            unsigned char index = (unsigned char)c;
            if(debug)
    	        std::cout << "ASCII (" << index << ")" << (int)index << std::endl;

            // Use vector lookup
            index = charMap[index];

            // 假設圖片是 16x16 的網格
            int colIndex = index % 16;
            int rowIndex = index / 16;

            SDL_Rect srcRect;
            srcRect.x = colIndex * charWidth;
            srcRect.y = rowIndex * charHeight;
            srcRect.w = charWidth;
            srcRect.h = charHeight;

            SDL_Rect dstRect;
            dstRect.x = cursorX;
            dstRect.y = cursorY;
            dstRect.w = displayW;
            dstRect.h = displayH;

            // 渲染這個字
            SDL_RenderCopy(renderer, fontTexture, &srcRect, &dstRect);

            // 游標向右移
            cursorX += displayW;
        }
	out_count+=1;
    }
};

#endif
