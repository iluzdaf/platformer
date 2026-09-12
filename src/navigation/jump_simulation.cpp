#include <cmath>
#include <vector>
#include "navigation/jump_simulation.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/abilities.hpp"
#include "input/input_intentions.hpp"
#include "navigation/jump_arc.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
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
    Abilities abilities(shortened);
    AbilityStates states;
    Observed observed;
    InputIntentions inputIntentions = holdingJumpAndRunning();

    observed.contacts.onGround = true;
    glm::vec2 takeOff = abilities.decide(PhysicsStep, inputIntentions, observed, states);
    if (takeOff.y >= 0.0f)
        return {};

    std::vector<glm::vec2> offsets{glm::vec2(0.0f)};
    glm::vec2 offset = takeOff * PhysicsStep;
    offsets.push_back(offset);

    observed.contacts.onGround = false;
    for (int step = 1; step < MaximumSteps; ++step)
    {
        offset += abilities.decide(PhysicsStep, inputIntentions, observed, states) * PhysicsStep;
        offsets.push_back(offset);

        if (offset.y >= 0.0f)
            return JumpArc{holdDuration, holdFraction, offsets};
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

JumpAttempt simulateJumpAgainst(
    const TileMap &tileMap,
    const ActorMotionData &motionData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction,
    float holdFraction)
{
    ActorMotionData shortened = releasedAfter(motionData, holdFraction);
    Abilities abilities(shortened);
    AbilityStates states;
    Observed observed;

    PhysicsBody physicsBody(physicsBodyData);
    physicsBody.setPosition(takeOffFeet - physicsBody.bottomCenterOffset());

    InputIntentions inputIntentions = holdingJumpAndRunning();
    inputIntentions.direction.x = direction;

    auto feet = [&] { return physicsBody.aabb().bottomCenter(); };

    JumpAttempt attempt;
    attempt.path.push_back(feet());

    observed.contacts.onGround = true;
    for (int step = 0; step < MaximumSteps; ++step)
    {
        physicsBody.setVelocity(abilities.decide(PhysicsStep, inputIntentions, observed, states));
        physicsBody.stepPhysics(PhysicsStep, tileMap);

        observed.contacts.onGround = physicsBody.contactWithGround(tileMap);
        observed.contacts.hitCeiling = physicsBody.contactWithCeiling(tileMap);
        observed.contacts.touchingLeftWall = physicsBody.contactWithLeftWall(tileMap);
        observed.contacts.touchingRightWall = physicsBody.contactWithRightWall(tileMap);
        observed.velocity = physicsBody.velocity();

        attempt.path.push_back(feet());
        attempt.steps = step + 1;

        if (step > 0 && observed.contacts.onGround)
        {
            float tileSize = static_cast<float>(tileMap.getTileSize());
            attempt.path.back().y = std::round(attempt.path.back().y / tileSize) * tileSize;

            attempt.landed = true;
            return attempt;
        }
    }

    JumpAttempt capped;
    capped.steps = attempt.steps;
    capped.capped = true;
    return capped;
}
