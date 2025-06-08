#include <cassert>
#include <algorithm>
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"

Camera2D::Camera2D(const Camera2DData &cameraData, int screenWidth, int screenHeight)
    : zoom(cameraData.zoom)
{
    assert(zoom > 0.0f && "zoom must be positive and non-zero");
    resize(screenWidth, screenHeight);
}

void Camera2D::follow(const glm::vec2 &target)
{
    float halfW = screenWidth / (2.0f * zoom);
    float halfH = screenHeight / (2.0f * zoom);
    float worldW = worldMax.x - worldMin.x;
    float worldH = worldMax.y - worldMin.y;
    bool clampX = worldW > screenWidth / zoom;
    bool clampY = worldH > screenHeight / zoom;

    if (clampX)
        position.x = std::clamp(target.x, worldMin.x + halfW, worldMax.x - halfW);
    else
        position.x = worldMin.x + worldW / 2.0f;

    if (clampY)
        position.y = std::clamp(target.y, worldMin.y + halfH, worldMax.y - halfH);
    else
        position.y = worldMin.y + worldH / 2.0f;
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
    glm::vec2 cameraPosition = position + shakeOffset;
    float halfW = screenWidth / (2.0f * zoom);
    float halfH = screenHeight / (2.0f * zoom);
    return glm::ortho(
        cameraPosition.x - halfW,
        cameraPosition.x + halfW,
        cameraPosition.y + halfH,
        cameraPosition.y - halfH);
}

void Camera2D::resize(float screenWidth, float screenHeight)
{
    assert(screenWidth > 0.0f && "screenWidth must be positive");
    assert(screenHeight > 0.0f && "screenHeight must be positive");
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}

void Camera2D::update(float deltaTime)
{
    shakeOffset = shake.getOffset(deltaTime);
}

void Camera2D::startShake(float duration, float magnitude)
{
    shake.start(duration, magnitude);
}

float Camera2D::getZoom() const
{
    return zoom;
}

glm::vec2 Camera2D::getTopLeftPosition() const
{
    return getPosition() - glm::vec2(screenWidth, screenHeight) / (2.0f * zoom);
}