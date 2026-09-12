#pragma once

class GracePeriod
{
public:
    explicit GracePeriod(float length);

    void start(float direction = 0.0f);
    void update(float deltaTime);
    void consume();
    bool running() const;
    float direction() const;

private:
    float length = 0.0f;
    float left = 0.0f;
    float startedDirection = 0.0f;
};
