#include "animations/sprite_animation.hpp"

SpriteAnimation::SpriteAnimation(std::vector<int> frames,
                                 float frameDuration,
                                 int frameWidth,
                                 int frameHeight,
                                 int textureWidth)
    : animation(std::move(frames), frameDuration),
      frameWidth(frameWidth),
      frameHeight(frameHeight),
      textureWidth(textureWidth) {}

void SpriteAnimation::update(float deltaTime)
{
    animation.update(deltaTime);
}

glm::vec2 SpriteAnimation::getUVStart() const
{
    int frame = animation.getCurrentFrame();
    int cols = textureWidth / frameWidth;
    int x = frame % cols;
    int y = frame / cols;

    return glm::vec2(
        static_cast<float>(x * frameWidth) / textureWidth,
        static_cast<float>(y * frameHeight) / textureWidth);
}

glm::vec2 SpriteAnimation::getUVEnd() const
{
    glm::vec2 uvStart = getUVStart();
    return uvStart + glm::vec2(
                         static_cast<float>(frameWidth) / textureWidth,
                         static_cast<float>(frameHeight) / textureWidth);
}