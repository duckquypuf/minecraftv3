#pragma once

#include <string>

namespace BlockFlags
{
    constexpr int SOLID = 0;
    constexpr int LIQUID = 1 << 0;
    constexpr int GRASSMAP = 1 << 1;  // tinted by grass.png
    constexpr int COLOURMAP = 1 << 2; // tinted by foliage.png
}

struct Block
{
    std::string name;
    bool isSolid = true;
    bool isTransparent = false;
    int flags = BlockFlags::SOLID;
    int textures[6];
    int overlayTextures[6] = {-1, -1, -1, -1, -1, -1};
    int colourmapX = 0;
    int colourmapY = 0;
};