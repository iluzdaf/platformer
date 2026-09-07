#pragma once

struct InputIntentions;
struct Observed;
struct ActorMotionState;

class Ability
{
public:
    virtual ~Ability() = default;
    virtual void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        ActorMotionState &state) = 0;
};