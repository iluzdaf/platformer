#pragma once

struct InputIntentions;
struct Observed;
struct AbilityStates;

class Ability
{
public:
    virtual ~Ability() = default;
    virtual void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states) = 0;
};