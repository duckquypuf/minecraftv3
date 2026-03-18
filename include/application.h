#pragma once

#include "window.h"
#include "renderer.h"
#include "player.h"

#include "world.h"
#include "chunk.h"

class Application
{
public:
    Window window = Window("Minecraft Clone V3", 1440, 900);
    Renderer renderer = Renderer("../src/shaders/vert.glsl", "../src/shaders/frag.glsl", true);
    Renderer waterRenderer = Renderer("../src/shaders/vert.glsl", "../src/shaders/transparentfrag.glsl");
    Renderer grassRenderer = Renderer("../src/shaders/foliagevert.glsl", "../src/shaders/grassfrag.glsl");
    Renderer foliageRenderer = Renderer("../src/shaders/foliagevert.glsl", "../src/shaders/foliagefrag.glsl");
    Player player = Player(glm::vec3(halfWorldWidth * chunkWidth, 100.f, halfWorldWidth * chunkWidth));

    World world = World(player.currentChunk);

    void run() {
        while(!glfwWindowShouldClose(window.window) && window.shouldRun)
        {
            window.processInput();

            if(glfwGetTime() - window.LAST_FRAME_TIME < window.TARGET_FRAME_TIME) {
                player.camera->processInput(&window.input, window.deltaTime);
                glfwPollEvents();
                continue;
            }

            window.LAST_FRAME_TIME = glfwGetTime();

            player.processInput(&window.input, window.deltaTime);

            if(player.currentChunk != player.lastChunk) {
                world.updateRenderDistance(player.currentChunk);
                player.lastChunk = player.currentChunk;
            }

            world.processChunkQueues();

            renderer.beginFrame();
            renderer.renderChunks(world, player.camera);
            waterRenderer.renderChunks(world, player.camera, LIQUID);
            grassRenderer.renderChunks(world, player.camera, GRASS);
            foliageRenderer.renderChunks(world, player.camera, FOLIAGE);

            player.update();
            window.drawImGui(player, world);
            window.update();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
};