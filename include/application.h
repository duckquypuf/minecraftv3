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
    Renderer renderer = Renderer("../src/shaders/vert.glsl", "../src/shaders/frag.glsl", "../src/shaders/skyvert.glsl", "../src/shaders/skyfrag.glsl", true);
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
            float rawTime = glfwGetTime() * 0.1f;
            float tilt = glm::radians(23.5f); // Earth's axial tilt, tweak to taste
            renderer.sunDir = glm::normalize(glm::vec3(
                sin(rawTime) * sin(tilt), // X offset from tilt
                sin(rawTime) * cos(tilt), // Y — height in sky
                cos(rawTime)              // Z — day/night cycle
            ));
            renderer.time = rawTime; // frag.glsl still needs -1 to 1 for brightness
            renderer.skyTime = rawTime;   // skyfrag.glsl gets the raw angle
            renderer.changeSun();
            renderer.renderChunks(world, player.camera);

            player.update();
            window.drawImGui(player, world, renderer);
            window.update();
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwTerminate();
    }
};