// src/Board.h
#ifndef BOARD_H
#define BOARD_H

#include <string>

enum TileType {
    TILE_START,
    TILE_PROPERTY,
    TILE_CHANCE,
    TILE_JAIL,
    TILE_UNKNOWN
};

struct Tile {
    int id;
    TileType type;
    std::string name;
    int price;
    int rent;
    int x, y; // 螢幕像素座標
    int w, h;
    int ownerId = -1;
};

#endif
