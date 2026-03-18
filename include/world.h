#pragma once

#include <unordered_map>
#include <queue>
#include <memory>

#include "voxelData.h"
#include "chunk.h"

class World {
public:
    std::unordered_map<long long, std::unique_ptr<Chunk>> chunks;

    std::queue<long long> chunkPopulationQueue;
    std::queue<long long> chunkMeshQueue;
    std::queue<long long> chunksToGenerateTrees;

    World(ChunkCoord& player) {
        /* SETUP NOISE */
        noise.SetFractalOctaves(3);
        noise.SetFrequency(0.01f);

        treeZoneNoise.SetFrequency(treeZoneFrequency);
        treePlacementNoise.SetFrequency(treePlacementFrequency);

        updateRenderDistance(player);
    }

    void updateRenderDistance(ChunkCoord& playerChunk) {
        for (auto it = chunks.begin(); it != chunks.end();) {
            ChunkCoord coord = ChunkCoord::fromHash(it->first);
            if (std::abs(coord.x - playerChunk.x) > renderDistance + 1 || 
                std::abs(coord.z - playerChunk.z) > renderDistance + 1) {
                it = chunks.erase(it);
            } else {
                it++;
            }
        }

        for(int x = playerChunk.x - renderDistance - 1; x <= playerChunk.x + renderDistance + 1; x++) {
            for(int z = playerChunk.z - renderDistance - 1; z <= playerChunk.z + renderDistance + 1; z++) {
                ChunkCoord coord(x, z);
                long long hash = coord.getHash();
                
                if(chunks.find(hash) == chunks.end()) {
                    chunkPopulationQueue.push(hash);
                }
            }
        }
    }

    void processChunkQueues() {
        // 1. Handle Population (One per frame)
        if (!chunkPopulationQueue.empty()) {
            long long hash = chunkPopulationQueue.front();
            chunkPopulationQueue.pop();

            if (chunks.find(hash) == chunks.end()) {
                chunks[hash] = std::make_unique<Chunk>(this, ChunkCoord::fromHash(hash));
            }
        } 
        if(!chunksToGenerateTrees.empty()) {
            long long hash = chunksToGenerateTrees.front();

            if(chunks[hash]->isPopulated) {
                chunksToGenerateTrees.pop();

                if(chunks.find(hash) != chunks.end()) {
                    Chunk* chunk = chunks[hash].get();


                    for(int x = 0; x < chunkWidth; x++) {
                        for(int z = 0; z < chunkWidth; z++) {
                            int y = chunk->getHeight(x, z) + 1;

                            if(chunk->voxelMap[x][y][z] != 0) continue;

                            uint16_t belowVoxelID = getVoxel(chunk, x, y - 1, z);

                            if (belowVoxelID == 2 || belowVoxelID == 1) {
                                // We can place a tree here
                                float treeChance = treeZoneNoise.GetNoise((float)chunk->coord.x * chunkWidth + x, (float)chunk->coord.z * chunkWidth + z);

                                if (treeChance > treeZoneThreshold) {
                                    float treePlacementChance = treePlacementNoise.GetNoise((float)chunk->coord.x * chunkWidth + x + 1000.f, (float)chunk->coord.z * chunkWidth + z + 1000.f);

                                    if (treePlacementChance > treePlacementThreshold) {
                                        GenerateTree(chunk, x, y, z);
                                    }
                                }
                            }
                        }
                    }

                    chunkMeshQueue.push(hash);
                }
            }
        } 
        if (!chunkMeshQueue.empty()) {
            long long hash = chunkMeshQueue.front();
            ChunkCoord c = ChunkCoord::fromHash(hash);

            // Check the 4 immediate neighbors
            bool neighborsReady = 
                chunks.count(ChunkCoord(c.x + 1, c.z).getHash()) &&
                chunks.count(ChunkCoord(c.x - 1, c.z).getHash()) &&
                chunks.count(ChunkCoord(c.x, c.z + 1).getHash()) &&
                chunks.count(ChunkCoord(c.x, c.z - 1).getHash());

            if (neighborsReady) {
                chunkMeshQueue.pop();
                chunks[hash]->generateMesh(this);
            } else {
                chunkMeshQueue.pop();
                chunkMeshQueue.push(hash);
            }
        }
    }

    void GenerateTree(Chunk* chunk, int x, int y, int z) {
        int treeHeight = treeHeightMin + (rand() % (treeHeightMax - treeHeightMin + 1));

        for(int i = 0; i < treeHeight; i++) {
            if(y + i < chunkHeight) {
                chunk->voxelMap[x][y + i][z] = 6; // Oak Log
            } else {
                return;
            }
        }

        for(int tx = -2; tx <= 2; tx++) {
            for(int tz = -2; tz <= 2; tz++) {
                if(tx+x < 0 || tx+x >= chunkWidth || tz+z < 0 || tz+z >= chunkWidth) continue;

                if(chunk->voxelMap[tx+x][y+treeHeight-2][tz+z] == 0) {
                    if(abs(tx) == 2 && abs(tz) == 2) {
                        if(rand() % 100 < 25) { // 25% chance for leaves to spawn in the corners
                            chunk->setVoxel(tx+x, y+treeHeight-2, tz+z, 7); // Oak Leaves
                        }
                    } else {
                        chunk->setVoxel(tx+x, y+treeHeight-2, tz+z, 7); // Oak Leaves
                    }
                }

                if(chunk->voxelMap[tx+x][y+treeHeight-3][tz+z] == 0) {
                    if(abs(tx) == 2 && abs(tz) == 2) {
                        if(rand() % 100 < 25) { // 25% chance for leaves to spawn in the corners
                            chunk->setVoxel(tx+x, y+treeHeight-3, tz+z, 7); // Oak Leaves
                        }
                    } else {
                        chunk->setVoxel(tx+x, y+treeHeight-3, tz+z, 7); // Oak Leaves
                    }
                }
            }
        }

        for(int tx = -1; tx <= 1; tx++) {
            for(int tz = -1; tz <= 1; tz++) {
                if(tx+x < 0 || tx+x >= chunkWidth || tz+z < 0 || tz+z >= chunkWidth) continue;

                if(chunk->voxelMap[tx+x][y+treeHeight-1][tz+z] == 0) {
                    chunk->setVoxel(tx+x, y+treeHeight-1, tz+z, 7); // Oak Leaves
                }
            }
        }

        chunk->setVoxel(x, y+treeHeight, z, 7);
        chunk->setVoxel(x-1, y+treeHeight, z, 7);
        chunk->setVoxel(x, y+treeHeight, z-1, 7);
        chunk->setVoxel(x+1, y+treeHeight, z, 7);
        chunk->setVoxel(x, y+treeHeight, z+1, 7);
    }

    uint16_t GenTerrainVoxel(Chunk* chunk, int x, int y, int z) {
        if (y == 0)
            return 3; // Bedrock
        if (y > minTerrainHeight + terrainHeight)
            return 0; // Air above terrain generation limit

        int height = chunk->getHeight(x, z);

        uint16_t voxelID = 0;

        /* TERRAIN GEN PASS */
        if(y == height && y == waterLevel)
            voxelID = 5; // Sand
        else if(y == height && y < waterLevel && y > waterLevel - 4)
            voxelID = 1; // Dirt
        else if(y == height && y <= waterLevel - 4)
            voxelID = 3; // Stone
        else if(y == height)
            voxelID = 2; // Grass Block
        else if(y > height && y <= waterLevel)
            voxelID = 4; // Water
        else if(y < height && y > height - 4)
            voxelID = 1; // Dirt
        else if(y < height)
            voxelID = 3; // Stone

        return voxelID;
    }

    uint16_t getVoxel(Chunk* chunk, int x, int y, int z) {
        if(x >= 0 && x < chunkWidth && y >= 0 && y < chunkHeight && z >= 0 && z < chunkWidth) {
            return chunk->voxelMap[x][y][z];
        } else {
            ChunkCoord newCoord = chunk->coord;

            if(x < 0) {
                newCoord.x -= 1;
                x += chunkWidth;
            } else if(x > chunkWidth - 1) {
                newCoord.x += 1;
                x %= chunkWidth;
            }
            
            if(z < 0) {
                newCoord.z -= 1;
                z += chunkWidth;
            } else if (z > chunkWidth - 1) {
                newCoord.z += 1;
                z %= chunkWidth;
            } 

            if(y < 0 || y >= chunkHeight) { // Y, no subchunks so just render it
                return 0;
            }
            
            if(newCoord.x < 0 || newCoord.x > worldWidth - 1 || newCoord.z < 0 || newCoord.z > worldWidth -1) return 0;

            long long hash = newCoord.getHash();

            auto it = chunks.find(hash);

            if (it == chunks.end()) return 0;

            return it->second->voxelMap[x][y][z];
        }
    }

    void setVoxel(int worldX, int worldY, int worldZ, uint16_t id) {
        if (worldY < 0 || worldY >= chunkHeight)
            return;

        // Convert world coords to Chunk Coord + Local Coord
        int chunkX = std::floor((float)worldX / chunkWidth);
        int chunkZ = std::floor((float)worldZ / chunkWidth);

        int localX = worldX - (chunkX * chunkWidth);
        int localZ = worldZ - (chunkZ * chunkWidth);

        long long hash = ChunkCoord(chunkX, chunkZ).getHash();
        if (chunks.count(hash))
            chunks[hash]->voxelMap[localX][worldY][localZ] = id;
    }
};