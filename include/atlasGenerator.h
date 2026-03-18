#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <cstring>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize2.h"

// Simple JSON parser for texture list
std::vector<std::string> parseTextureList(const std::string &jsonPath)
{
    std::vector<std::string> textures;
    std::ifstream file(jsonPath);

    if (!file.is_open())
    {
        std::cerr << "ERROR: Could not open JSON file: " << jsonPath << std::endl;
        return textures;
    }

    std::string line;
    bool inArray = false;

    while (std::getline(file, line))
    {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);

        if (line.find("[") != std::string::npos)
            inArray = true;

        if (line.find("]") != std::string::npos)
            break;

        if (inArray && line.find("\"") != std::string::npos)
        {
            size_t first = line.find("\"");
            size_t last = line.find("\"", first + 1);

            if (first != std::string::npos && last != std::string::npos)
            {
                std::string texture = line.substr(first + 1, last - first - 1);

                // Skip if it's not a filename (doesn't have an extension)
                if (texture.find(".") != std::string::npos)
                {
                    textures.push_back(texture);
                }
            }
        }
    }

    file.close();
    return textures;
}

// Resize image to 16x16
unsigned char *resizeImage(unsigned char *input, int inW, int inH, int channels)
{
    const int targetSize = 16;
    unsigned char *output = new unsigned char[targetSize * targetSize * channels];

    stbir_resize_uint8_linear(
        input, inW, inH, 0,
        output, targetSize, targetSize, 0,
        (stbir_pixel_layout)channels);

    return output;
}

class AtlasGenerator
{
public:
    static bool generateAtlas(const std::string &jsonPath,
                              const std::string &textureDir,
                              const std::string &outputPath)
    {
        const int ATLAS_SIZE = 256;
        const int TILE_SIZE = 16;
        const int TILES_PER_ROW = ATLAS_SIZE / TILE_SIZE; // 16
        const int CHANNELS = 4;                           // RGBA

        std::cout << "=== Texture Atlas Generator ===" << std::endl;

        // Parse JSON file
        std::vector<std::string> textureFiles = parseTextureList(jsonPath);

        if (textureFiles.empty())
        {
            std::cerr << "ERROR: No textures found in JSON file" << std::endl;
            return false;
        }

        std::cout << "Found " << textureFiles.size() << " textures in JSON" << std::endl;

        if (textureFiles.size() > TILES_PER_ROW * TILES_PER_ROW)
        {
            std::cerr << "WARNING: Too many textures (" << textureFiles.size()
                      << "), max is " << (TILES_PER_ROW * TILES_PER_ROW) << std::endl;
        }

        // Print texture list with IDs
        std::cout << "\nTexture ID mapping:" << std::endl;
        for (size_t i = 0; i < textureFiles.size(); i++)
        {
            std::cout << "  ID " << i << ": " << textureFiles[i] << std::endl;
        }
        std::cout << std::endl;

        // Create atlas buffer (RGBA)
        unsigned char *atlas = new unsigned char[ATLAS_SIZE * ATLAS_SIZE * CHANNELS];
        memset(atlas, 0, ATLAS_SIZE * ATLAS_SIZE * CHANNELS); // Initialize to transparent

        // Track loaded textures
        int successCount = 0;
        int failCount = 0;

        // Load and place each texture
        for (size_t i = 0; i < textureFiles.size() && i < TILES_PER_ROW * TILES_PER_ROW; i++)
        {
            std::string texPath = textureDir + "/" + textureFiles[i];

            int width, height, channels;
            unsigned char *texData = stbi_load(texPath.c_str(), &width, &height, &channels, CHANNELS);

            if (!texData)
            {
                std::cerr << "ERROR: Failed to load texture [ID " << i << "]: " << textureFiles[i] << std::endl;
                std::cerr << "       Tried path: " << texPath << std::endl;
                failCount++;
                continue;
            }

            std::cout << "✓ Loaded [ID " << i << "]: " << textureFiles[i] << " (" << width << "x" << height << ")" << std::endl;

            // Resize if necessary
            unsigned char *processedData = texData;
            bool needsResize = (width != TILE_SIZE || height != TILE_SIZE);

            if (needsResize)
            {
                std::cout << "  → Resizing to 16x16..." << std::endl;
                processedData = resizeImage(texData, width, height, CHANNELS);
            }

            // Calculate position in atlas
            int tileX = i % TILES_PER_ROW;
            int tileY = i / TILES_PER_ROW;
            int atlasX = tileX * TILE_SIZE;
            int atlasY = tileY * TILE_SIZE;

            std::cout << "  → Atlas position: (" << atlasX << ", " << atlasY << ")" << std::endl;

            // Copy texture data to atlas
            for (int y = 0; y < TILE_SIZE; y++)
            {
                for (int x = 0; x < TILE_SIZE; x++)
                {
                    int srcIdx = (y * TILE_SIZE + x) * CHANNELS;
                    int dstIdx = ((atlasY + y) * ATLAS_SIZE + (atlasX + x)) * CHANNELS;

                    // Copy RGBA
                    atlas[dstIdx + 0] = processedData[srcIdx + 0]; // R
                    atlas[dstIdx + 1] = processedData[srcIdx + 1]; // G
                    atlas[dstIdx + 2] = processedData[srcIdx + 2]; // B
                    atlas[dstIdx + 3] = processedData[srcIdx + 3]; // A
                }
            }

            // Cleanup
            if (needsResize)
                delete[] processedData;
            stbi_image_free(texData);
            successCount++;
        }

        // Print summary
        std::cout << "\n=== Summary ===" << std::endl;
        std::cout << "Successfully loaded: " << successCount << " textures" << std::endl;
        std::cout << "Failed to load: " << failCount << " textures" << std::endl;

        // Create output directory if it doesn't exist
        std::filesystem::path outPath(outputPath);
        std::filesystem::create_directories(outPath.parent_path());

        // Save atlas
        int success = stbi_write_png(outputPath.c_str(), ATLAS_SIZE, ATLAS_SIZE,
                                     CHANNELS, atlas, ATLAS_SIZE * CHANNELS);

        delete[] atlas;

        if (success)
        {
            std::cout << "\n✓ SUCCESS: Atlas saved to " << outputPath << std::endl;
            std::cout << "Atlas size: " << ATLAS_SIZE << "x" << ATLAS_SIZE << " pixels" << std::endl;
            std::cout << "Tile size: " << TILE_SIZE << "x" << TILE_SIZE << " pixels" << std::endl;
            std::cout << "Grid: " << TILES_PER_ROW << "x" << TILES_PER_ROW << " tiles" << std::endl;
            return true;
        }
        else
        {
            std::cerr << "\n✗ ERROR: Failed to save atlas to " << outputPath << std::endl;
            return false;
        }
    }
};