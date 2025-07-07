#include <algorithm>
#include <stdexcept>
#include "cameras/camera2d.hpp"

Camera2D::Camera2D(
    Camera2DData cameraData, 
    int windowWidth, 
    int windowHeight)
{
    setZoom(cameraData.zoom);

    resize(windowWidth, windowHeight);
}

void Camera2D::follow(glm::vec2 target)
{
    float halfW = windowWidth / (2.0f * zoom);
    float halfH = windowHeight / (2.0f * zoom);
    float worldW = worldMax.x - worldMin.x;
    float worldH = worldMax.y - worldMin.y;
    bool clampX = worldW > windowWidth / zoom;
    bool clampY = worldH > windowHeight / zoom;

    if (clampX)
        position.x = std::clamp(target.x, worldMin.x + halfW, worldMax.x - halfW);
    else
        position.x = worldMin.x + worldW / 2.0f;

    if (clampY)
        position.y = std::clamp(target.y, worldMin.y + halfH, worldMax.y - halfH);
    else
        position.y = worldMin.y + worldH / 2.0f;
}

void Camera2D::setWorldBounds(glm::vec2 min, glm::vec2 max)
{
    if (min.x >= max.x)
        throw std::invalid_argument("min.x must be less than max.x");
    if (min.y >= max.y)
        throw std::invalid_argument("min.y must be less than max.y");

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
    float halfW = windowWidth / (2.0f * zoom);
    float halfH = windowHeight / (2.0f * zoom);
    return glm::ortho(
        cameraPosition.x - halfW,
        cameraPosition.x + halfW,
        cameraPosition.y + halfH,
        cameraPosition.y - halfH);
}

void Camera2D::resize(int width, int height)
{
    if (width <= 0)
        throw std::invalid_argument("width must be positive");
    if (height <= 0)
        throw std::invalid_argument("height must be positive");

    windowWidth = static_cast<float>(width);
    windowHeight = static_cast<float>(height);
}

void Camera2D::update(float deltaTime)
{
    shakeOffset = shake.getOffset(deltaTime);
}

void Camera2D::startShake(float duration, float magnitude)
{
    if (duration <= 0)
        throw std::invalid_argument("Shake duration must be greater than 0");
    if (magnitude <= 0)
        throw std::invalid_argument("Shake magnitude must be greater than 0");

    shake.start(duration, magnitude);
}

float Camera2D::getZoom() const
{
    return zoom;
}

glm::vec2 Camera2D::getTopLeftPosition() const
{
    return getPosition() - glm::vec2(windowWidth, windowHeight) / (2.0f * zoom);
}

void Camera2D::setZoom(float newZoom)
{
    if (newZoom <= 0.0f)
        throw std::invalid_argument("Camera zoom must be positive and non-zero");

    zoom = newZoom;
}

glm::vec2 Camera2D::worldToCameraRelative(glm::vec2 worldPosition) const
{
    return worldPosition - getTopLeftPosition();
}

glm::vec2 Camera2D::getWindowSize() const
{
    return glm::vec2(windowWidth, windowHeight);
}

bool Camera2D::shaking() const
{
    return shake.active;
}