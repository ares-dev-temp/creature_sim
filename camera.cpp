#include "camera.h"

Camera::Camera( glm::vec3 pos, float aspectRatio ){
    position = pos;
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    forward = glm::vec3(0.0f, 0.0f, -1.0f);
    perspectiveMatrix = glm::perspective( glm::radians(60.0f), aspectRatio, 0.1f, 1000.0f );
}

void Camera::update_viewMatrix(){
    glm::vec3 dirFront = position + forward;
    viewMatrix = glm::lookAt( position, dirFront, up );
}

