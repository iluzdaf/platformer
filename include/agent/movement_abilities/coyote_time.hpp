#pragma once

struct CoyoteTime
{
    explicit CoyoteTime(float duration = 0.1f);

    void update(bool eligibleForCoyoteTime, float dt);
    bool isCoyoteAvailable() const;
    void consume();
    void setDuration(float duration);

    float coyoteDuration = 0.1f;
    float coyoteTimer = 0.0f;
};
