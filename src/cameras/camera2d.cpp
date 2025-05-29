#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include <algorithm>
#include "cameras/camera2d.hpp"

Camera2D::Camera2D(const Camera2DData &cameraData)
    : screenWidth(cameraData.width), screenHeight(cameraData.height), zoom(cameraData.zoom)
{
    assert(screenWidth > 0.0f && "screenWidth must be positive");
    assert(screenHeight > 0.0f && "screenHeight must be positive");
    assert(zoom > 0.0f && "zoom must be positive and non-zero");
}

void Camera2D::follow(const glm::vec2 &target)
{
    float halfW = screenWidth / (2.0f * zoom);
    float halfH = screenHeight / (2.0f * zoom);
    position.x = std::clamp(target.x, worldMin.x + halfW, worldMax.x - halfW);
    position.y = std::clamp(target.y, worldMin.y + halfH, worldMax.y - halfH);
}

void Camera2D::setWorldBounds(const glm::vec2 &min, const glm::vec2 &max)
{
    assert(min.x < max.x && "worldMin.x must be less than worldMax.x");
    assert(min.y < max.y && "worldMin.y must be less than worldMax.y");

    worldMin = min;
    worldMax = max;
}

glm::vec2 Camera2D::getPosition() const
{
    return position;
}

glm::mat4 Camera2D::getProjection() const
{
    float halfW = screenWidth / (2.0f * zoom);
    float halfH = screenHeight / (2.0f * zoom);
    return glm::ortho(
        position.x - halfW,
        position.x + halfW,
        position.y + halfH,
        position.y - halfH);
}