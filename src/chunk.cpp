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

void Chunk::generateMesh(World *world)
{
    vertices.clear();
    vertexCount = 0;

    for (int y = 0; y < chunkHeight; y++)
    {
        for (int x = 0; x < chunkWidth; x++)
        {
            for (int z = 0; z < chunkWidth; z++)
            {
                const Block *self = blocks[voxelMap[x][y][z]];
                if (voxelMap[x][y][z] == 0)
                    continue;

                for (int f = 0; f < 6; f++)
                {
                    int fx = x + faceChecks[f].x;
                    int fy = y + faceChecks[f].y;
                    int fz = z + faceChecks[f].z;

                    const Block *neighbour = blocks[world->getVoxel(this, fx, fy, fz)];

                    if (!neighbour->isTransparent) // neighbour is opaque so you cant see this face
                        continue;

                    // Liquid faces: skip same-liquid neighbours
                    if ((self->flags & BlockFlags::LIQUID) &&
                        (neighbour->flags & BlockFlags::LIQUID) &&
                        neighbour == self)
                        continue;

                    for (int p = 0; p < 6; p++)
                    {
                        vertices.emplace_back(cubeVertices[faces[f][p]].x + x);
                        vertices.emplace_back(cubeVertices[faces[f][p]].y + y);
                        vertices.emplace_back(cubeVertices[faces[f][p]].z + z);
                        vertices.emplace_back(faceChecks[f].x);
                        vertices.emplace_back(faceChecks[f].y);
                        vertices.emplace_back(faceChecks[f].z);
                        vertices.emplace_back(self->textures[f]);
                        vertices.emplace_back(self->colourmapX);
                        vertices.emplace_back(self->colourmapY);
                        vertices.emplace_back(self->flags);
                    }
                    vertexCount += 6;
                }
            }
        }
    }

    if (VAO == 0)
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
    }

    constexpr int STRIDE = 10 * sizeof(int);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int), vertices.data(), GL_STATIC_DRAW);

    // location 0: position
    glVertexAttribIPointer(0, 3, GL_INT, STRIDE, (void *)(0));
    glEnableVertexAttribArray(0);

    // location 1: normal
    glVertexAttribIPointer(1, 3, GL_INT, STRIDE, (void *)(3 * sizeof(int)));
    glEnableVertexAttribArray(1);

    // location 2: textureID
    glVertexAttribIPointer(2, 1, GL_INT, STRIDE, (void *)(6 * sizeof(int)));
    glEnableVertexAttribArray(2);

    // location 3: colourmapX
    glVertexAttribIPointer(3, 1, GL_INT, STRIDE, (void *)(7 * sizeof(int)));
    glEnableVertexAttribArray(3);

    // location 4: colourmapY
    glVertexAttribIPointer(4, 1, GL_INT, STRIDE, (void *)(8 * sizeof(int)));
    glEnableVertexAttribArray(4);

    // location 5: flags
    glVertexAttribIPointer(5, 1, GL_INT, STRIDE, (void *)(9 * sizeof(int)));
    glEnableVertexAttribArray(5);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    isMeshed = true;
}