#pragma once

struct InputIntentions;
struct ActorMotionState;

class Ability
{
public:
    virtual ~Ability() = default;
    virtual void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state) = 0;
};