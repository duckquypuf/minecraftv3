#pragma once

#include <string>

enum BlockType {
    SOLID,
    LIQUID,
    GRASS,
    FOLIAGE
};

struct Block {
    std::string name;
    bool isSolid = true;
    bool isTransparent = false;
    BlockType type = SOLID;
    int textures[6];

    bool operator==(const Block& other) const {
        return name == other.name;
    }
};

struct Grass : Block {
    int colourmapX;
    int colourmapY;

    Grass(const Block& block, int colourmapX, int colourmapY) : Block(block), colourmapX(colourmapX), colourmapY(colourmapY) {
        type = GRASS;
    }
};

struct Foliage : Block {
    int colourmapX;
    int colourmapY;

    Foliage(const Block& block, int colourmapX, int colourmapY) : Block(block), colourmapX(colourmapX), colourmapY(colourmapY) {
        type = FOLIAGE;
        isTransparent = true; 
    }
};