#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <unordered_map>

#include "imageEdit.h"

#include "shader.h"
#include "atlasGenerator.h"

#include "camera.h"
#include "world.h"
#include "chunk.h"

class Renderer
{
public:
    unsigned int atlasTexture;
    unsigned int grassmapTexture;
    unsigned int colourmapTexture;

    Renderer(const char *vertPath, const char *fragPath, bool genAtlas = false)
    {
        std::cout << "Generating biome-based textures..." << std::endl;

        apply_multiply("../textures/grass_top.png", "../textures/grass_top.png", 1.f, 1.f, 1.f);

        apply_multiply("../textures/grass_side_overlay.png", "../textures/grass_side_overlay.png", 1.f, 1.f, 1.f);
        apply_overlay("../textures/grass_side.png", "../textures/grass_side_overlay.png", "../textures/grass_side_plains.png");

        apply_multiply("../textures/oak_leaves.png", "../textures/oak_leaves.png", 1.f, 1.f, 1.f);

        if (genAtlas) {
            std::cout << "Generating texture atlas..." << std::endl;
            bool success = AtlasGenerator::generateAtlas(
                "../textures/textures.json",
                "../textures",
                "../atlas/atlas_256x256.png"
            );

            if (!success)
            {
                std::cerr << "WARNING: Atlas generation failed, using existing atlas if available" << std::endl;
            }
        }

        shader = new Shader(vertPath, fragPath);

        atlasTexture = loadTexture("../atlas/atlas_256x256.png");
        grassmapTexture = loadTexture("../textures/grass.png");
        colourmapTexture = loadTexture("../textures/foliage.png");
    }

    void beginFrame()
    {
        glClearColor(0.47f, 0.66f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void renderChunks(World &world, Camera *cam)
    {
        shader->use();

        // Bind all three textures once — shader selects based on flags
        shader->setInt("atlasTex", 0);
        shader->setInt("grassmapTex", 1);
        shader->setInt("colourmapTex", 2);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlasTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, grassmapTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, colourmapTexture);

        for (auto &pair : world.chunks)
            pair.second->renderChunk(shader, cam->GetViewMatrix(), cam->GetProjectionMatrix());
    }

private:
    Shader *shader;

    unsigned int loadTexture(const char *path)
    {
        if (!std::filesystem::exists(path))
        {
            std::cout << "ERROR: File does not exist at path: " << path << std::endl;
            return 0;
        }

        unsigned int id;
        glGenTextures(1, &id);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

        if (data)
        {
            std::cout << "Texture loaded successfully: " << width << "x" << height << " with " << nrComponents << " components" << std::endl;
            GLenum format;

            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "STB Image failed to load. Reason: " << stbi_failure_reason() << std::endl;
            std::cout << "Path attempted: " << path << std::endl;
            stbi_image_free(data);
            return 0;
        }
        return id;
    }
};