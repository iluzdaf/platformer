#pragma once

struct InputIntentions;
struct Observed;
struct Decided;

class Ability
{
public:
    virtual ~Ability() = default;
    virtual void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided) = 0;
};