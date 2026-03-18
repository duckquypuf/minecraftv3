#pragma once

#include <glm/glm.hpp>

#include "FastNoiseLite.h"

#include "voxel.h"

inline const int worldWidth = 100;
inline const int halfWorldWidth = worldWidth / 2;

inline int renderDistance = 10;

inline const int chunkWidth = 16;
inline const int chunkHeight = 128;

inline const int waterLevel = 63;

inline const int minTerrainHeight = 64;
inline const int terrainHeight = 10;

inline const int seed = 1738;
inline FastNoiseLite noise(seed);

inline const int treeZoneSeed = seed + 1738;
inline FastNoiseLite treeZoneNoise(treeZoneSeed);

inline const int treePlacementSeed = treeZoneSeed + 1738;
inline FastNoiseLite treePlacementNoise(treePlacementSeed);
inline const float treeZoneFrequency = 0.01f;
inline const float treePlacementFrequency = 0.5f;

inline const float treeZoneThreshold = 0.3f;
inline const float treePlacementThreshold = 0.94f;
inline const int treeHeightMin = 4;
inline const int treeHeightMax = 7;

static const Block air = {
    .name = "Air",
    .isSolid = false,
    .isTransparent = true,
    .textures = {0, 0, 0, 0, 0, 0}
};
static const Block dirt = {
    .name = "Dirt",
    .textures = {0, 0, 0, 0, 0, 0}
};
static const Block grassBlock = {
    .name = "Grass_Block",
    .textures = {1, 1, 2, 0, 1, 1},
    .flags = BlockFlags::GRASSMAP,
    .colourmapX = 51,
    .colourmapY = 153
};
static const Block stone = {
    .name = "Stone",
    .textures = {3, 3, 3, 3, 3, 3}
};
static const Block water = {
    .name = "Water",
    .isSolid = false,
    .isTransparent = true,
    .flags = BlockFlags::LIQUID,
    .textures = {4, 4, 4, 4, 4, 4}
};
static const Block sand = {
    .name = "Sand",
    .textures = {5, 5, 5, 5, 5, 5}
};
static const Block oakLog = {
    .name="Oak Log",
    .textures={6, 6, 7, 7, 6, 6}
};
static const Block oakLeaves = {
    .name="Oak Leaves", 
    .isTransparent = true,
    .textures={8, 8, 8, 8, 8, 8},
    .flags = BlockFlags::COLOURMAP,
    .colourmapX = 50, 
    .colourmapY = 173
};


inline const Block *blocks[] = {
    &air,
    &dirt,
    &grassBlock,
    &stone,
    &water,
    &sand,
    &oakLog,
    &oakLeaves,
};
inline const glm::ivec3 cubeVertices[8] = {
//   X  Y  Z
    {0, 0, 0}, // 0
    {0, 0, 1}, // 1
    {1, 0, 1}, // 2
    {1, 0, 0}, // 3
    {0, 1, 0}, // 4
    {0, 1, 1}, // 5
    {1, 1, 1}, // 6
    {1, 1, 0}  // 7
};

inline const int faces[6][6] = {
    {0, 3, 4, 4, 3, 7}, // Front
    {2, 1, 6, 6, 1, 5}, // Back
    {4, 7, 5, 5, 7, 6}, // Top
    {1, 2, 0, 0, 2, 3}, // Bottom
    {1, 0, 5, 5, 0, 4}, // Left
    {3, 2, 7, 7, 2, 6}, // Right
};

inline const glm::ivec3 faceChecks[6] = {
    {0, 0, -1},
    {0, 0, 1},
    {0, 1, 0},
    {0, -1, 0},
    {-1, 0, 0},
    {1, 0, 0}
};