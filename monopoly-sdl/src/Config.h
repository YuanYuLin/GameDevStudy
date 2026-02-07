#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

struct GameConfig {
    int windowWidth = 800;  // 預設值
    int windowHeight = 600; // 預設值
    int playerCount = 2;    // 預設值
    int startMoney = 1500;  // 預設值
};

class ConfigLoader {
public:
    static GameConfig LoadConfig(const std::string& filename) {
        GameConfig config;
        std::ifstream file(filename);
        
        if (!file.is_open()) {
            std::cerr << "Warning: Could not open settings file. Using defaults." << std::endl;
            return config;
        }

        std::string line;
        while (std::getline(file, line)) {
            // 跳過空行或註解
            if (line.empty() || line[0] == '#') continue;

            // 尋找 '=' 的位置
            size_t delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos) continue;

            // 切割 Key 和 Value
            std::string key = line.substr(0, delimiterPos);
            std::string valueStr = line.substr(delimiterPos + 1);

            // 簡單的字串比對與賦值
            try {
                int value = std::stoi(valueStr);

                if (key == "WINDOW_WIDTH") config.windowWidth = value;
                else if (key == "WINDOW_HEIGHT") config.windowHeight = value;
                else if (key == "PLAYER_COUNT") {
                    config.playerCount = value;
                    if (config.playerCount < 2) config.playerCount = 2; // 防呆
                    if (config.playerCount > 6) config.playerCount = 6; // 上限
                }
                else if (key == "START_MONEY") config.startMoney = value;
                
            } catch (...) {
                std::cerr << "Error parsing line: " << line << std::endl;
            }
        }

        file.close();
        return config;
    }
};

#endif
