#pragma once

#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/ability.hpp"

struct ActorMotionData;
struct AbilityStates;
struct InputIntentions;
struct Observed;

class Abilities
{
public:
    explicit Abilities(const ActorMotionData &motionData);
    glm::vec2 decide(
        float deltaTime,
        const InputIntentions &inputIntentions,
        const Observed &observed,
        AbilityStates &states);

private:
    std::vector<std::unique_ptr<Ability>> abilities;
};