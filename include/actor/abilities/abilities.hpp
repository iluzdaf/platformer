#pragma once

#include <vector>
#include <memory>
#include "actor/abilities/ability.hpp"

struct ActorMotionData;
struct Decided;
struct InputIntentions;
struct Observed;

class Abilities
{
public:
    explicit Abilities(const ActorMotionData &motionData);
    void decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        Decided &decided);

private:
    std::vector<std::unique_ptr<Ability>> abilities;
};