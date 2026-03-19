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
    unsigned int liquidVAO = 0, liquidVBO = 0;

    std::vector<int> vertices;
    std::vector<int> liquidVertices;
    int vertexCount = 0;
    int liquidVertexCount = 0;

    bool isMeshed = false;
    bool isPopulated = false;
    bool treesGenerated = false;

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

    void renderOpaque(Shader* shader, const glm::mat4& view, const glm::mat4& projection) {
        if (vertexCount == 0) return;
        setMatrices(shader, view, projection);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

    void renderLiquid(Shader* shader, const glm::mat4& view, const glm::mat4& projection) {
        if (liquidVertexCount == 0) return;
        setMatrices(shader, view, projection);
        glBindVertexArray(liquidVAO);
        glDrawArrays(GL_TRIANGLES, 0, liquidVertexCount);
        glBindVertexArray(0);
    }

    void renderChunk(Shader *shader, const glm::mat4 &view, const glm::mat4 &projection)
    {
        if (vertexCount == 0)
            return;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(coord.x * chunkWidth, 0.f, coord.z * chunkWidth));

        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
        glBindVertexArray(0);
    }

private:
    void setMatrices(Shader *shader, const glm::mat4 &view, const glm::mat4 &projection) {
        glm::mat4 model = glm::translate(glm::mat4(1.0f),
                                         glm::vec3(coord.x * chunkWidth, 0.f, coord.z * chunkWidth));
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);
    }
};