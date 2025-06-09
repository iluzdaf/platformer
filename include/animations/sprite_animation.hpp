#pragma once
#include <glm/glm.hpp>
#include "animations/frame_animation.hpp"
class SpriteAnimationData;

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
    int frameWidth = 0;
    int frameHeight = 0;
    int textureWidth = 0;
};