#pragma once

#include <glm/glm.hpp>

#include "input.h"
#include "camera.h"

#include "chunkCoord.h"

class Player {
public:
    glm::vec3 pos;
    glm::ivec3 blockPos;
    glm::ivec3 chunkPos;
    ChunkCoord currentChunk;
    ChunkCoord lastChunk;

    Camera* camera;

    float moveSpeed = 10.f;

    Player(glm::vec3 spawnPos) {
        pos = spawnPos;
        camera = new Camera();
        camera->player = this;
        blockPos = glm::ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
        chunkPos = glm::ivec3(blockPos.x % 16, blockPos.y, blockPos.z % 16);
        currentChunk.x = blockPos.x / 16;
        currentChunk.z = blockPos.z / 16;
    }

    void update() {
        lastChunk.x = currentChunk.x;
        lastChunk.z = currentChunk.z;
        blockPos = glm::ivec3(floor(pos.x), floor(pos.y), floor(pos.z));
        chunkPos = glm::ivec3(blockPos.x % 16, blockPos.y, blockPos.z % 16);
        currentChunk.x = blockPos.x / 16;
        currentChunk.z = blockPos.z / 16;
    }

    void processInput(Input* input, float dt) {
        glm::vec3 flatForward = glm::normalize(glm::vec3(camera->forward.x, 0.f, camera->forward.z));

        if (input->w)
            pos += flatForward * moveSpeed * dt;
        if (input->a)
            pos -= camera->right * moveSpeed * dt;
        if (input->s)
            pos -= flatForward * moveSpeed * dt;
        if (input->d)
            pos += camera->right * moveSpeed * dt;

        if (input->space)
            pos += camera->worldUp * moveSpeed * dt;

        if (input->l_shift)
            pos -= camera->worldUp * moveSpeed * dt;

        camera->processInput(input, dt);
    }
};