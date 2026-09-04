#pragma once
class Shader;

class ScreenTransition
{
public:
    ScreenTransition();
    ~ScreenTransition();
    ScreenTransition(const ScreenTransition &) = delete;
    ScreenTransition &operator=(const ScreenTransition &) = delete;
    void start(float duration, bool fadeIn = true);
    void update(float deltaTime);
    void draw(const Shader &shader) const;
    bool isActive() const;
    float getAlpha() const;

private:
    unsigned int vertexArrayObject, vertexBufferObject;
    float duration = 1.0f, timer = 0.0f, alpha = 0.0f;
    bool active = false, fadeIn = true;

    void initQuad();
};