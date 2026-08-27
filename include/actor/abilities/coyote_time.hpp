#pragma once

class CoyoteTime
{
public:
    explicit CoyoteTime(float duration = 0.1f);

    void update(bool eligibleForCoyoteTime, float dt);
    bool isCoyoteAvailable() const;
    void consume();

private:
    float coyoteDuration = 0.1f;
    float coyoteTimer = 0.0f;
};
