#pragma once
#include <glm/glm.hpp>
#include "animations/sprite_animation_data.hpp"
#include "animations/frame_animation.hpp"

class SpriteAnimation
{
public:
    SpriteAnimation() = default;
    SpriteAnimation(const SpriteAnimationData &spriteAnimationData);
    void update(float deltaTime);
    glm::vec2 getUVStart() const;
    glm::vec2 getUVEnd() const;

private:
    FrameAnimation frameAnimation;
    int frameWidth = 32;
    int frameHeight = 32;
    int textureWidth = 128;
};