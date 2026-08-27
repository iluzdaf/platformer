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

JumpArc simulateJumpArc(const ActorMotionData &motionData, float holdFraction)
{
    ActorMotionData shortened = releasedAfter(motionData, holdFraction);
    float holdDuration = shortened.jumpAbilityData ? shortened.jumpAbilityData->jumpDuration : 0.0f;
    AbilitySystem abilitySystem(shortened);
    ActorMotionState state;
    InputIntentions inputIntentions = holdingJumpAndRunning();

    state.contacts.onGround = true;
    abilitySystem.applyMovement(SimulationTimeStep, inputIntentions, state);
    if (state.targetVelocity.y >= 0.0f)
        return {};

    std::vector<glm::vec2> offsets{glm::vec2(0.0f)};
    glm::vec2 offset = state.targetVelocity * SimulationTimeStep;
    offsets.push_back(offset);

    state.contacts.onGround = false;
    for (int step = 1; step < MaximumSteps; ++step)
    {
        abilitySystem.applyMovement(SimulationTimeStep, inputIntentions, state);
        offset += state.targetVelocity * SimulationTimeStep;
        offsets.push_back(offset);

        if (offset.y >= 0.0f)
            return JumpArc{holdDuration, offsets};
    }

    return {};
}

std::vector<JumpArc> simulateJumpArcs(const ActorMotionData &motionData)
{
    std::vector<JumpArc> arcs;

    for (float holdFraction : HoldFractions)
    {
        JumpArc arc = simulateJumpArc(motionData, holdFraction);
        if (!arc.offsets.empty())
            arcs.push_back(arc);
    }

    return arcs;
}
