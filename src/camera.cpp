#include "player.h"
#include "camera.h"

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(player->pos, player->pos + forward, up);
}