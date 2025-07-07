#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include "cameras/camera_shake.hpp"
#include "cameras/camera2d_data.hpp"

class Camera2D
{
public:
    Camera2D(
        Camera2DData cameraData,
        int windowWidth,
        int windowHeight);
    void follow(glm::vec2 target);
    void setWorldBounds(
        glm::vec2 min,
        glm::vec2 max);
    glm::mat4 getProjection() const;
    glm::vec2 getPosition() const;
    void resize(int windowWidth, int windowHeight);
    void update(float deltaTime);
    void startShake(float duration, float magnitude);
    float getZoom() const;
    glm::vec2 getTopLeftPosition() const;
    void setZoom(float zoom);
    glm::vec2 worldToCameraRelative(glm::vec2 worldPosition) const;
    glm::vec2 getWindowSize() const;
    bool shaking() const;

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