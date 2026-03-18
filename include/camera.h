#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

#include "input.h"

#include "voxelData.h"

class Player;

class Camera
{
public:
    Player* player;

    glm::vec3 forward;
    glm::vec3 up;
    glm::vec3 worldUp;
    glm::vec3 right;

    Camera()
    {
        forward = glm::vec3(0.0f, 0.0f, 1.0f);
        worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        up = worldUp;

        yaw = 90.0f;
        pitch = 0.0f;
        lastX = 1440.0f / 2.0f;
        lastY = 900.0f / 2.0f;
        firstMouse = true;

        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix();

    glm::mat4 GetProjectionMatrix()
    {
        return glm::perspective(glm::radians(FOV), 1440.0f / 900.0f, 0.1f, 1000.0f);
    }

    void processInput(Input* input, float dt) {
        yaw += input->mouseX * dt * mouseSensitivity;
        pitch -= input->mouseY * dt * mouseSensitivity;

        if(pitch < -89.f) pitch = -89.f;
        if(pitch > 89.f) pitch = 89.f;

        updateCameraVectors();
    }

private:
    float mouseSensitivity = 10.f;
    float FOV = 90.0f;

    bool firstMouse;
    float lastX, lastY;
    float yaw, pitch;

    void updateCameraVectors()
    {
        glm::vec3 newForward;

        newForward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newForward.y = sin(glm::radians(pitch));
        newForward.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        forward = glm::normalize(newForward);

        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::cross(right, forward);
    }
};