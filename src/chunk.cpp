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
}

void Chunk::generateMesh(World *world)
{
    vertices.clear();
    liquidVertices.clear();
    vertexCount = 0;
    liquidVertexCount = 0;

    for (int y = 0; y < chunkHeight; y++)
    {
        for (int x = 0; x < chunkWidth; x++)
        {
            for (int z = 0; z < chunkWidth; z++)
            {
                const Block *self = blocks[voxelMap[x][y][z]];
                if (voxelMap[x][y][z] == 0)
                    continue;

                bool isLiquid = (self->flags & BlockFlags::LIQUID);

                for (int f = 0; f < 6; f++)
                {
                    int fx = x + faceChecks[f].x;
                    int fy = y + faceChecks[f].y;
                    int fz = z + faceChecks[f].z;

                    const Block *neighbour = blocks[world->getVoxel(this, fx, fy, fz)];

                    if ((self->flags & BlockFlags::LIQUID) && (neighbour->flags & BlockFlags::LIQUID))
                        continue;

                    if (!neighbour->isTransparent)
                        continue;

                    auto &target = isLiquid ? liquidVertices : vertices;
                    auto &targetCount = isLiquid ? liquidVertexCount : vertexCount;

                    for (int p = 0; p < 6; p++)
                    {
                        target.emplace_back(cubeVertices[faces[f][p]].x + x);
                        target.emplace_back(cubeVertices[faces[f][p]].y + y);
                        target.emplace_back(cubeVertices[faces[f][p]].z + z);
                        target.emplace_back(faceChecks[f].x);
                        target.emplace_back(faceChecks[f].y);
                        target.emplace_back(faceChecks[f].z);
                        target.emplace_back(self->textures[f]);
                        target.emplace_back(self->overlayTextures[f]);
                        target.emplace_back(self->colourmapX);
                        target.emplace_back(self->colourmapY);
                        target.emplace_back(self->flags);
                    }
                    targetCount += 6;
                }
            }
        }
    }

    constexpr int STRIDE = 11 * sizeof(int);

    // Helper lambda to avoid repeating the attrib pointer setup
    auto setupVAO = [&](unsigned int &vao, unsigned int &vbo, std::vector<int> &verts)
    {
        if (vao == 0)
        {
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &vbo);
        }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(int), verts.data(), GL_STATIC_DRAW);
        glVertexAttribIPointer(0, 3, GL_INT, STRIDE, (void *)(0)); // pos
        glEnableVertexAttribArray(0);
        glVertexAttribIPointer(1, 3, GL_INT, STRIDE, (void *)(3 * sizeof(int))); // normal
        glEnableVertexAttribArray(1);
        glVertexAttribIPointer(2, 1, GL_INT, STRIDE, (void *)(6 * sizeof(int))); // textureID
        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(3, 1, GL_INT, STRIDE, (void *)(7 * sizeof(int))); // overlayTextureID
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(4, 1, GL_INT, STRIDE, (void *)(8 * sizeof(int))); // colourmapX
        glEnableVertexAttribArray(4);
        glVertexAttribIPointer(5, 1, GL_INT, STRIDE, (void *)(9 * sizeof(int))); // colourmapY
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(6, 1, GL_INT, STRIDE, (void *)(10 * sizeof(int))); // flags
        glEnableVertexAttribArray(6);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    };

    setupVAO(VAO, VBO, vertices);
    setupVAO(liquidVAO, liquidVBO, liquidVertices);

    isMeshed = true;
}