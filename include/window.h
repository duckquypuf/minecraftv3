#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>
#include <sstream>
#include <iomanip>

/* THIRD PARTY INCLUDES */
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "input.h"
#include "player.h"
#include "world.h"

class Window
{
public:
    int screenWidth, screenHeight;
    GLFWwindow* window;
    std::string windowName;

    float deltaTime = 0.f;
    float lastFrame = 0.f;

    float lowFps = 0.f;
    float highFps = 0.f;
    float avgFps = 0.f;
    int avgFpsSamples = 0;

    float avgFpsTimer = 0.f;
    float avgFpsInterval = 1.f;

    Input input;

    bool shouldRun = true;

    float TARGET_FPS = 144.f;
    float TARGET_FRAME_TIME = 1.f / TARGET_FPS;
    float LAST_FRAME_TIME = 0.f;
    
    Window(std::string windowName, int screenWidth, int screenHeight)
    {
        this->screenWidth = screenWidth;
        this->screenHeight = screenHeight;
        this->windowName = windowName;
        initOpenGL();
    }

    void initOpenGL()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(screenWidth, screenHeight, windowName.c_str(), NULL, NULL);

        if (window == NULL)
        {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwTerminate();
            return;
        }

        glfwSwapInterval(0); // Disable VSync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410 core");

        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void drawImGui(Player& player, World& world) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Information");
        ImGui::Text("FPS: %.0f", 1.0f / deltaTime);
        ImGui::Text("LOW: %.0f", lowFps);
        ImGui::Text("HIGH: %.0f", highFps);
        ImGui::Text("AVG: %.0f", avgFps);
        ImGui::Text("Player Position: (%.2f, %.2f, %.2f)", player.pos.x, player.pos.y, player.pos.z);
        ImGui::Text("Block Position: (%d, %d, %d)", player.blockPos.x, player.blockPos.y, player.blockPos.z);
        ImGui::Text("Chunk Position: (%d, %d, %d)", player.chunkPos.x, player.chunkPos.y, player.chunkPos.z);
        ImGui::Text("Current Chunk: (%d, %d)", player.currentChunk.x, player.currentChunk.z);
        ImGui::End();

        ImGui::Begin("World");
        ImGui::Text("Render Distance: %d", renderDistance);
        ImGui::Text("Loaded Chunks: %zu", world.chunks.size());
        ImGui::Text("Chunks to Populate: %zu", world.chunkPopulationQueue.size());
        ImGui::Text("Chunks to Generate Trees: %zu", world.chunksToGenerateTrees.size());
        ImGui::Text("Chunks to Mesh: %zu", world.chunkMeshQueue.size());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void update() {
        // --- Calculate Delta Time ---
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (avgFpsTimer < avgFpsInterval) {
            lowFps = std::min(lowFps, 1.f / deltaTime);
            highFps = std::max(highFps, 1.f / deltaTime);
            avgFpsTimer += deltaTime;
            avgFpsSamples++;
        } else {
            avgFps = avgFpsSamples / avgFpsTimer;
            avgFpsTimer = 0.f;
            lowFps = 1.0f / deltaTime;
            highFps = 1.0f / deltaTime;
            avgFpsSamples = 0;
        }
    }

    void processInput() {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) shouldRun = false;

        if(glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !input.f3) {
            mouseLocked = !mouseLocked;

            input.f3 = true;

            if(mouseLocked) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                firstMouse = false;
            } else glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        input.f3 = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;

        input.w = glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
        input.a = glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS;
        input.s = glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
        input.d = glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS;
        input.space = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        input.l_shift = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        if(firstMouse) {
            firstMouse = false;
            lastX = mouseX;
            lastY = mouseY;
        } else {
            input.mouseX = mouseX - lastX;
            input.mouseY = mouseY - lastY;

            lastX = mouseX;
            lastY = mouseY;
        }
    }

private:
    double lastX = 0., lastY = 0.;
    bool firstMouse = true;
    bool mouseLocked = true;
};