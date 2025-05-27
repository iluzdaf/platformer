#include <cassert>
#include "animations/sprite_animation.hpp"

SpriteAnimation::SpriteAnimation(const SpriteAnimationData &spriteAnimationData)
    : frameAnimation(spriteAnimationData.frameAnimationData),
      frameWidth(spriteAnimationData.frameWidth),
      frameHeight(spriteAnimationData.frameHeight),
      textureWidth(spriteAnimationData.textureWidth)
{
    assert(frameWidth > 0);
    assert(frameHeight > 0);
    assert(textureWidth > 0);
    assert(textureWidth % frameWidth == 0 && "textureWidth must be divisible by frameWidth");
    assert(textureWidth % frameHeight == 0 && "textureWidth must be divisible by frameHeight");
}

void SpriteAnimation::update(float deltaTime)
{
    frameAnimation.update(deltaTime);
}

glm::vec2 SpriteAnimation::getUVStart() const
{
    if (frameWidth == 0 || frameHeight == 0 || textureWidth == 0)
        return glm::vec2(0.0f, 0.0f);

    int frame = frameAnimation.getCurrentFrame();
    int cols = textureWidth / frameWidth;
    int x = frame % cols;
    int y = frame / cols;

    return glm::vec2(
        static_cast<float>(x * frameWidth) / textureWidth,
        static_cast<float>(y * frameHeight) / textureWidth);
}

glm::vec2 SpriteAnimation::getUVEnd() const
{
    if (frameWidth == 0 || frameHeight == 0 || textureWidth == 0)
        return glm::vec2(1.0f, 1.0f);

    glm::vec2 uvStart = getUVStart();
    return uvStart + glm::vec2(
                         static_cast<float>(frameWidth) / textureWidth,
                         static_cast<float>(frameHeight) / textureWidth);
}