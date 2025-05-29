#pragma once
#include <glm/glm.hpp>
#include "cameras/camera2d_data.hpp"

class Camera2D
{
public:
    explicit Camera2D(const Camera2DData &cameraData);
    void follow(const glm::vec2 &target);
    void setWorldBounds(const glm::vec2 &min, const glm::vec2 &max);
    glm::mat4 getProjection() const;
    glm::vec2 getPosition() const;

private:
    float screenWidth, screenHeight;
    float zoom;
    glm::vec2 position;
    glm::vec2 worldMin = glm::vec2(0.0f);
    glm::vec2 worldMax = glm::vec2(10000.0f);
};