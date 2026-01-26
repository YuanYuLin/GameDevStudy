#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "Board.h"

const float MOVE_SPEED = 200.0f;

class Player {
public:
    int id;
    int money;
    int currentTileIndex;
    float x, y;
    
    // 狀態
    bool isMoving;
    int stepsRemaining;
    int targetTileIndex;

    // 新增：玩家顏色
    Uint8 r, g, b; 

    // 建構子新增顏色參數
    Player(int _id, int startIndex, const std::vector<Tile>& map, Uint8 _r, Uint8 _g, Uint8 _b) 
        : id(_id), money(1500), currentTileIndex(startIndex), isMoving(false), stepsRemaining(0),
          r(_r), g(_g), b(_b) {
        
        if (!map.empty()) {
            x = map[startIndex].x;
            y = map[startIndex].y;
        }
    }

    void RollDice(int diceNumber) {
        if (isMoving) return;
        stepsRemaining = diceNumber;
        isMoving = true;
    }

    void Pay(int amount) {
        money -= amount;
        if (money < 0) money = 0;
    }

    void Receive(int amount) {
        money += amount;
    }

    void Update(float deltaTime, const std::vector<Tile>& map) {
        if (!isMoving) return;

        if (stepsRemaining > 0) {
            int nextIndex = (currentTileIndex + 1) % map.size();
            float targetX = map[nextIndex].x;
            float targetY = map[nextIndex].y;
            
            float dirX = targetX - x;
            float dirY = targetY - y;
            float distance = std::sqrt(dirX*dirX + dirY*dirY);

            if (distance < 2.0f) {
                x = targetX;
                y = targetY;
                currentTileIndex = nextIndex;
                stepsRemaining--;
                if (stepsRemaining <= 0) isMoving = false;
            } else {
                x += (dirX / distance) * MOVE_SPEED * deltaTime;
                y += (dirY / distance) * MOVE_SPEED * deltaTime;
            }
        }
    }

    void Render(SDL_Renderer* renderer) {
        // 為了避免兩人重疊看不見，我們可以根據 ID 微調一下顯示位置
        int offsetX = (id - 1) * 5; 
        int offsetY = (id - 1) * 5;

        SDL_Rect rect = { (int)x + offsetX + 5, (int)y + offsetY + 5, 20, 20 };
        
        // 使用玩家自己的顏色
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderFillRect(renderer, &rect);
        
        // 畫個黑框區分
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &rect);
    }
};

#endif
