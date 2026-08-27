#include "navigation/jump_arc.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/ability_system.hpp"
#include "input/input_intentions.hpp"

namespace
{
    constexpr float SimulationTimeStep = 0.01f;
    constexpr int MaximumSteps = 1000;
    constexpr float HoldFractions[] = {1.0f, 0.75f, 0.5f, 0.25f};

    InputIntentions holdingJumpAndRunning()
    {
        InputIntentions inputIntentions;
        inputIntentions.direction.x = 1.0f;
        inputIntentions.jumpRequested = true;
        inputIntentions.jumpHeld = true;
        return inputIntentions;
    }

    ActorMotionData releasedAfter(const ActorMotionData &motionData, float holdFraction)
    {
        ActorMotionData shortened = motionData;
        if (shortened.jumpAbilityData)
            shortened.jumpAbilityData->jumpDuration *= holdFraction;
        return shortened;
    }
}

std::vector<glm::vec2> simulateJumpArc(
    const ActorMotionData &motionData,
    float holdFraction)
{
    AbilitySystem abilitySystem(releasedAfter(motionData, holdFraction));
    ActorMotionState state;
    InputIntentions inputIntentions = holdingJumpAndRunning();

    state.contacts.onGround = true;
    abilitySystem.applyMovement(SimulationTimeStep, inputIntentions, state);
    if (state.targetVelocity.y >= 0.0f)
        return {};

    std::vector<glm::vec2> arc{glm::vec2(0.0f)};
    glm::vec2 offset = state.targetVelocity * SimulationTimeStep;
    arc.push_back(offset);

    state.contacts.onGround = false;
    for (int step = 1; step < MaximumSteps; ++step)
    {
        abilitySystem.applyMovement(SimulationTimeStep, inputIntentions, state);
        offset += state.targetVelocity * SimulationTimeStep;
        arc.push_back(offset);

        if (offset.y >= 0.0f)
            return arc;
    }

    return {};
}

std::vector<std::vector<glm::vec2>> simulateJumpArcs(const ActorMotionData &motionData)
{
    std::vector<std::vector<glm::vec2>> arcs;

    for (float holdFraction : HoldFractions)
    {
        std::vector<glm::vec2> arc = simulateJumpArc(motionData, holdFraction);
        if (!arc.empty())
            arcs.push_back(arc);
    }

    return arcs;
}
