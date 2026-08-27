#pragma once

class ActionBuffer
{
public:
    explicit ActionBuffer(float duration = 0.1f);

    void press();
    void update(float dt);
    bool isBuffered() const;
    void consume();

private:
    float bufferDuration = 0.1f;
    float bufferTimer = 0.0f;
};
