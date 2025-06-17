#include <algorithm>
#include "cameras/camera2d.hpp"
#include "cameras/camera2d_data.hpp"

Camera2D::Camera2D(
    const Camera2DData &cameraData, 
    int windowWidth, 
    int windowHeight)
{
    setZoom(cameraData.zoom);

    resize(windowWidth, windowHeight);
}

void Camera2D::follow(const glm::vec2 &target)
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

void Camera2D::setWorldBounds(const glm::vec2 &min, const glm::vec2 &max)
{
    if (min.x >= max.x)
        throw std::invalid_argument("worldMin.x must be less than worldMax.x");
    if (min.y >= max.y)
        throw std::invalid_argument("worldMin.y must be less than worldMax.y");

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

void Camera2D::resize(int windowWidth, int windowHeight)
{
    if (windowWidth <= 0.0f)
        throw std::invalid_argument("windowWidth must be positive");
    if (windowHeight <= 0.0f)
        throw std::invalid_argument("windowHeight must be positive");

    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
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

void Camera2D::setZoom(float zoom)
{
    if (zoom <= 0.0f)
        throw std::invalid_argument("Camera zoom must be positive and non-zero");

    this->zoom = zoom;
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