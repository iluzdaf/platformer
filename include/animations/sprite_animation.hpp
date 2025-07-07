#pragma once
#include <glm/glm.hpp>
#include "animations/frame_animation.hpp"

struct SpriteAnimationData;

class SpriteAnimation
{
public:
    explicit SpriteAnimation(const SpriteAnimationData &spriteAnimationData);
    void update(float deltaTime);
    glm::vec2 getUVStart() const;
    glm::vec2 getUVEnd() const;
    void reset();

private:
    FrameAnimation frameAnimation;
    int frameWidth = 0,
        frameHeight = 0,
        textureWidth = 0;
};