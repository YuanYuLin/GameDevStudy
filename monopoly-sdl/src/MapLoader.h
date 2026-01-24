// src/MapLoader.h
#ifndef MAPLOADER_H
#define MAPLOADER_H

#include "Board.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class MapLoader {
private:
    static TileType StringToTileType(const std::string& str) {
        if (str == "START") return TILE_START;
        if (str == "PROPERTY") return TILE_PROPERTY;
        if (str == "CHANCE") return TILE_CHANCE;
        if (str == "JAIL") return TILE_JAIL;
        return TILE_UNKNOWN;
    }

public:
    static std::vector<Tile> LoadMap(const std::string& filename) {
        std::vector<Tile> tiles;
        std::ifstream file(filename);

        if (!file.is_open()) {
            std::cerr << "Error: Could not open map file: " << filename << std::endl;
            return tiles;
        }

        std::string line;
        while (std::getline(file, line)) {
            // 跳過空行或註解
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            Tile tile;
            std::string typeStr;
            
            // 格式: ID Type Name Price Rent X Y
            ss >> tile.id >> typeStr >> tile.name >> tile.price >> tile.rent >> tile.x >> tile.y;

            tile.type = StringToTileType(typeStr);
            tiles.push_back(tile);
        }
        file.close();
        return tiles;
    }
};

#endif
