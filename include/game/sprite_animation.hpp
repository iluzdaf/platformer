#pragma once

#include "game/frame_animation.hpp"
#include <glm/glm.hpp>
#include <vector>

class SpriteAnimation
{
public:
    SpriteAnimation() = default;
    SpriteAnimation(std::vector<int> frames,
                    float frameDuration,
                    int frameWidth,
                    int frameHeight,
                    int textureWidth);
    void update(float deltaTime);
    glm::vec2 getUVStart() const;
    glm::vec2 getUVEnd() const;

private:
    FrameAnimation animation;
    int frameWidth = 32;
    int frameHeight = 32;
    int textureWidth = 128;
};