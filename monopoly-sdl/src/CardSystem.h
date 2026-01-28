#ifndef CARDSYSTEM_H
#define CARDSYSTEM_H

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib> // for rand()

enum CardEffect {
    EFFECT_MONEY, // 加減錢
    EFFECT_MOVE,  // 傳送到某格
    EFFECT_UNKNOWN
};

struct Card {
    CardEffect type;
    int value; // 金額 或 目標格子ID
    std::string description;
};

class CardSystem {
private:
    std::vector<Card> deck;

    CardEffect StringToEffect(const std::string& str) {
        if (str == "MONEY") return EFFECT_MONEY;
        if (str == "MOVE") return EFFECT_MOVE;
        return EFFECT_UNKNOWN;
    }

public:
    // 載入卡片資料
    bool LoadCards(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string typeStr;
            Card card;
            
            ss >> typeStr >> card.value >> card.description;
            card.type = StringToEffect(typeStr);
            
            deck.push_back(card);
        }
        file.close();
        return true;
    }

    // 隨機抽一張卡 (不從牌堆移除，模擬無限抽)
    const Card* DrawRandomCard() {
        if (deck.empty()) return nullptr;
        int index = std::rand() % deck.size();
        return &deck[index];
    }
};

#endif
