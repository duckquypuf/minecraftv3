#pragma once

#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "voxelData.h"

#include "chunkCoord.h"

class World;

class Chunk {
public:
    uint16_t voxelMap[chunkWidth][chunkHeight][chunkWidth];

    int heightMap[chunkWidth][chunkWidth] = {0};
    bool heightMapGenerated[chunkWidth][chunkWidth] = {false};

    ChunkCoord coord;

    /* MESH */
    unsigned int VAO = 0, VBO = 0;
    unsigned int waterVAO = 0, waterVBO = 0;
    unsigned int grassVAO = 0, grassVBO = 0;
    unsigned int foliageVAO = 0, foliageVBO = 0;

    std::vector<int> vertices;
    int vertexCount = 0;

    std::vector<int> waterVertices;
    int waterVertexCount = 0;

    std::vector<int> grassVertices;
    int grassVertexCount = 0;

    std::vector<int> foliageVertices;
    int foliageVertexCount = 0;

    bool isMeshed = false;
    bool isPopulated = false;

    Chunk(World* world, ChunkCoord _coord) : coord(_coord) {
        populateVoxelMap(world);
    }

    int getHeight(int x, int z) {
        if (heightMapGenerated[x][z]) {
            return heightMap[x][z];
        } else {
            float height01 = noise.GetNoise((float)coord.x * chunkWidth + x, (float)coord.z * chunkWidth + z);
            int height = minTerrainHeight + (int)(height01 * terrainHeight);
            heightMap[x][z] = height;
            heightMapGenerated[x][z] = true;
            return height;
        }
    }

    void populateVoxelMap(World* world);

    bool isVoxelTransparent(int x, int y, int z) {
        if (x < 0 || x > chunkWidth - 1 || z < 0 || z > chunkWidth - 1|| y < 0 || y > chunkHeight - 1)
            return true;

        return blocks[voxelMap[x][y][z]]->isTransparent;
    }

    void generateMesh(World* world);

    void setVoxel(int x, int y, int z, uint16_t id) {
        if (x < 0 || x > chunkWidth - 1 || z < 0 || z > chunkWidth - 1|| y < 0 || y > chunkHeight - 1)
            return;

        voxelMap[x][y][z] = id;
    }

    void renderChunk(Shader* shader, const glm::mat4 &view, const glm::mat4 &projection, BlockType type = SOLID) {
        if (vertexCount == 0)
            return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(coord.x * chunkWidth, 0.f, coord.z * chunkWidth));

        if(type == LIQUID) {
            model = glm::translate(model, glm::vec3(0.f, -0.1f, 0.f));
        }

        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        if(type == LIQUID){
            glBindVertexArray(waterVAO);
            glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);
            glBindVertexArray(0);
            return;
        } else if(type == GRASS) {
            glBindVertexArray(grassVAO);
            glDrawArrays(GL_TRIANGLES, 0, grassVertexCount);
            glBindVertexArray(0);
            return;
        } else if(type == FOLIAGE) {
            glBindVertexArray(foliageVAO);
            glDrawArrays(GL_TRIANGLES, 0, foliageVertexCount);
            glBindVertexArray(0);
            return;
        } else {
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, vertexCount);
            glBindVertexArray(0);
        }
    }
};