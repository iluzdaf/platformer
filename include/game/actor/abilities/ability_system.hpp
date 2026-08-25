#pragma once

#include <vector>
#include <memory>
#include "game/actor/abilities/ability.hpp"

struct ActorMotionData;
struct ActorMotionState;
struct InputIntentions;

class AbilitySystem
{
public:
    explicit AbilitySystem(const ActorMotionData &motionData);
    void applyMovement(
        float deltaTime,
        const InputIntentions &inputIntentions,
        ActorMotionState &state);

private:
    std::vector<std::unique_ptr<Ability>> abilities;
};