#pragma once
class Shader;

class ScreenTransition
{
public:
    ScreenTransition();
    ~ScreenTransition();
    void start(float duration, bool fadeIn = true);
    void update(float deltaTime);
    void draw(const Shader &shader);

    bool isActive() const;

private:
    unsigned int VAO, VBO;

    float duration;
    float timer;
    float alpha;
    bool active;
    bool fadeIn;

    void initQuad();
};