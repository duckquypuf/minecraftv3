#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "world.h"
#include "chunk.h"

#include "voxelData.h"

void Chunk::populateVoxelMap(World* world) {
    for (int y = 0; y < chunkHeight; y++) {
        for (int x = 0; x < chunkWidth; x++) {
            for (int z = 0; z < chunkWidth; z++) {
                voxelMap[x][y][z] = world->GenTerrainVoxel(this, x, y, z);
            }
        }
    }

    isPopulated = true;
    world->chunksToGenerateTrees.push(coord.getHash());
}

void Chunk::generateMesh(World* world) {
    vertices.clear();
    waterVertices.clear();
    grassVertices.clear();
    foliageVertices.clear();
    vertexCount = 0;
    waterVertexCount = 0;
    grassVertexCount = 0;
    foliageVertexCount = 0;

    for (int y = 0; y < chunkHeight; y++) {
        for (int x = 0; x < chunkWidth; x++) {
            for (int z = 0; z < chunkWidth; z++) {
                for (int f = 0; f < 6; f++) {
                    int fx = x + faceChecks[f].x;
                    int fy = y + faceChecks[f].y;
                    int fz = z + faceChecks[f].z;

                    const Block* neighbour = blocks[world->getVoxel(this, fx, fy, fz)];
                    const Block* self = blocks[voxelMap[x][y][z]];

                    if (neighbour->isTransparent && voxelMap[x][y][z] != 0) {
                        if(self->type == LIQUID && neighbour->type == LIQUID && neighbour == self) {
                            continue;
                        }

                        if(self->type == LIQUID) {
                            for(int p = 0; p < 6; p++) {
                                waterVertices.emplace_back(cubeVertices[faces[f][p]].x + x);
                                waterVertices.emplace_back(cubeVertices[faces[f][p]].y + y);
                                waterVertices.emplace_back(cubeVertices[faces[f][p]].z + z);
                                waterVertices.emplace_back(faceChecks[f].x);
                                waterVertices.emplace_back(faceChecks[f].y);
                                waterVertices.emplace_back(faceChecks[f].z);
                                waterVertices.emplace_back(self->textures[f]);
                            }
                            waterVertexCount += 6;
                        } else if(self->type == GRASS) {
                            const Grass *grassBlock = static_cast<const Grass*>(self);

                            for (int p = 0; p < 6; p++) {
                                grassVertices.emplace_back(cubeVertices[faces[f][p]].x + x);
                                grassVertices.emplace_back(cubeVertices[faces[f][p]].y + y);
                                grassVertices.emplace_back(cubeVertices[faces[f][p]].z + z);
                                grassVertices.emplace_back(faceChecks[f].x);
                                grassVertices.emplace_back(faceChecks[f].y);
                                grassVertices.emplace_back(faceChecks[f].z);
                                grassVertices.emplace_back(self->textures[f]);
                                grassVertices.emplace_back(grassBlock->colourmapX);
                                grassVertices.emplace_back(grassBlock->colourmapY);
                            }
                            grassVertexCount += 6;
                        } else if(self->type == FOLIAGE) {
                            const Foliage* foliageBlock = static_cast<const Foliage*>(self);

                            for(int p = 0; p < 6; p++) {
                                foliageVertices.emplace_back(cubeVertices[faces[f][p]].x + x);
                                foliageVertices.emplace_back(cubeVertices[faces[f][p]].y + y);
                                foliageVertices.emplace_back(cubeVertices[faces[f][p]].z + z);
                                foliageVertices.emplace_back(faceChecks[f].x);
                                foliageVertices.emplace_back(faceChecks[f].y);
                                foliageVertices.emplace_back(faceChecks[f].z);
                                foliageVertices.emplace_back(self->textures[f]);
                                foliageVertices.emplace_back(foliageBlock->colourmapX);
                                foliageVertices.emplace_back(foliageBlock->colourmapY);
                            }
                            foliageVertexCount += 6;
                        } else {
                            for(int p = 0; p < 6; p++) {
                                vertices.emplace_back(cubeVertices[faces[f][p]].x + x);
                                vertices.emplace_back(cubeVertices[faces[f][p]].y + y);
                                vertices.emplace_back(cubeVertices[faces[f][p]].z + z);
                                vertices.emplace_back(faceChecks[f].x);
                                vertices.emplace_back(faceChecks[f].y);
                                vertices.emplace_back(faceChecks[f].z);
                                vertices.emplace_back(self->textures[f]);
                            }
                            vertexCount += 6;
                        }
                    }
                }
            }
        }
    }

    if (VAO == 0) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 3, GL_INT, 7 * sizeof(int), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 3, GL_INT, 7 * sizeof(int), (void *)(sizeof(int) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_INT, 7 * sizeof(int), (void *)(sizeof(int) * 6));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (waterVAO == 0) {
        glGenVertexArrays(1, &waterVAO);
        glGenBuffers(1, &waterVBO);
    }

    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(int), waterVertices.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 3, GL_INT, 7 * sizeof(int), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 3, GL_INT, 7 * sizeof(int), (void *)(sizeof(int) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_INT, 7 * sizeof(int), (void *)(sizeof(int) * 6));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (grassVAO == 0)
    {
        glGenVertexArrays(1, &grassVAO);
        glGenBuffers(1, &grassVBO);
    }

    glBindVertexArray(grassVAO);
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, grassVertices.size() * sizeof(int), grassVertices.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 3, GL_INT, 9 * sizeof(int), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 3, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 6));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 2, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 7));
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(4, 2, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 8));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (foliageVAO == 0)
    {
        glGenVertexArrays(1, &foliageVAO);
        glGenBuffers(1, &foliageVBO);
    }

    glBindVertexArray(foliageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, foliageVBO);
    glBufferData(GL_ARRAY_BUFFER, foliageVertices.size() * sizeof(int), foliageVertices.data(), GL_STATIC_DRAW);

    glVertexAttribIPointer(0, 3, GL_INT, 9 * sizeof(int), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(1, 3, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribIPointer(2, 1, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 6));
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(3, 2, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 7));
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(4, 2, GL_INT, 9 * sizeof(int), (void *)(sizeof(int) * 8));
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    isMeshed = true;
}