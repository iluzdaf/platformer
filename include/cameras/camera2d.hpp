#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "cameras/camera_shake.hpp"
class Camera2DData;

class Camera2D
{
public:
    Camera2D(
        const Camera2DData &cameraData,
        int windowWidth,
        int windowHeight);
    void follow(const glm::vec2 &target);
    void setWorldBounds(
        const glm::vec2 &min,
        const glm::vec2 &max);
    glm::mat4 getProjection() const;
    glm::vec2 getPosition() const;
    void resize(int windowWidth, int windowHeight);
    void update(float deltaTime);
    void startShake(float duration, float magnitude);
    float getZoom() const;
    glm::vec2 getTopLeftPosition() const;
    void setZoom(float zoom);

private:
    float windowWidth = 800,
          windowHeight = 600,
          zoom = 1;
    glm::vec2 position = glm::vec2(0.0f, 0.0f),
              worldMin = glm::vec2(0.0f),
              worldMax = glm::vec2(10000.0f),
              shakeOffset = glm::vec2(0.0f);
    CameraShake shake;
};