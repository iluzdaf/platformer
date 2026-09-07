#pragma once

#include <vector>
#include <memory>
#include "actor/abilities/ability.hpp"

struct ActorMotionData;
struct ActorMotionState;
struct InputIntentions;
struct Observed;

class AbilitySystem
{
public:
    explicit AbilitySystem(const ActorMotionData &motionData);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        ActorMotionState &state);

private:
    std::vector<std::unique_ptr<Ability>> abilities;
};