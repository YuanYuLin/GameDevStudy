#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>
#include <vector>
#include <cmath>
#include <iostream>
#include "Board.h"

// 移動速度 (像素/秒)
const float MOVE_SPEED = 200.0f;

class Player {
public:
    int id;
    int money; // 新增：金錢
    int currentTileIndex;   // 邏輯上的位置 (第幾格)
    float x, y;             // 視覺上的位置 (浮點數用於平滑移動)
    
    // 移動狀態控制
    bool isMoving;
    int stepsRemaining;     // 還剩下幾步要走 (例如擲出 3，走完一格剩 2)
    int targetTileIndex;    // 當前正在前往的「下一格」

    Player(int _id, int startIndex, const std::vector<Tile>& map) 
        : id(_id), money(1500), currentTileIndex(startIndex), isMoving(false), stepsRemaining(0) {
        
        // 初始化位置在起點
        if (!map.empty()) {
            x = map[startIndex].x;
            y = map[startIndex].y;
        }
    }

    // 擲骰子並開始移動
    void RollDice(int diceNumber) {
        if (isMoving) return; // 如果正在移動，忽略輸入

        std::cout << "Player rolled: " << diceNumber << std::endl;
        stepsRemaining = diceNumber;
        isMoving = true;
    }

// 新增：交易功能
    void Pay(int amount) {
        money -= amount;
        if (money < 0) money = 0; // 簡單處理，這裡先不做破產邏輯
    }

    void Receive(int amount) {
        money += amount;
    }
    // 每幀更新 (處理移動動畫)
    void Update(float deltaTime, const std::vector<Tile>& map) {
        if (!isMoving) return;

        // 1. 決定目標：如果還沒設定目標，就設為「下一格」
        if (stepsRemaining > 0) {
            int nextIndex = (currentTileIndex + 1) % map.size();
            
            // 取得目標座標
            float targetX = map[nextIndex].x;
            float targetY = map[nextIndex].y;

            // 2. 計算方向向量
            float dirX = targetX - x;
            float dirY = targetY - y;
            float distance = std::sqrt(dirX*dirX + dirY*dirY);

            // 3. 移動邏輯
            if (distance < 2.0f) {
                // A. 距離夠近了，視為「到達」該格
                x = targetX;
                y = targetY;
                currentTileIndex = nextIndex;
                stepsRemaining--; // 消耗一步

                // 如果步數走完了，停止移動
                if (stepsRemaining <= 0) {
                    isMoving = false;
                    std::cout << "Arrived at Tile " << currentTileIndex << std::endl;
                    // 這裡未來可以觸發「買地/過路費」事件
                }
            } else {
                // B. 還沒到，繼續往目標移動
                // 正規化向量 (Normalize) 並乘以速度與時間
                x += (dirX / distance) * MOVE_SPEED * deltaTime;
                y += (dirY / distance) * MOVE_SPEED * deltaTime;
            }
        }
    }

    // 渲染玩家
    void Render(SDL_Renderer* renderer) {
        // 畫一個紅色的實心方塊代表玩家
        SDL_Rect rect = { (int)x, (int)y, 20, 20 }; // 玩家比格子小一點 (20x20)
        
        // 為了讓玩家居中，我們稍微調整顯示位置 (假設格子是 40x40)
        rect.x += 10; 
        rect.y += 10;

        SDL_SetRenderDrawColor(renderer, 255, 50, 50, 255); // 紅色
        SDL_RenderFillRect(renderer, &rect);
    }
};

#endif
