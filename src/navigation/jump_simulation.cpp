#include <cmath>
#include <optional>
#include <vector>
#include "navigation/jump_simulation.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/mover.hpp"
#include "actor/observed.hpp"
#include "input/input_intentions.hpp"
#include "navigation/footing.hpp"
#include "navigation/input_program.hpp"
#include "navigation/jump_arc.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body_data.hpp"
#include "tile_map/tile_map.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr int MaximumSteps = 1000;
    constexpr int MaximumSettlingSteps = 30;
    constexpr float Settle = 0.5f;
    constexpr float HoldFractions[] = {1.0f, 0.75f, 0.5f, 0.25f};

    float restingOn(const TileMap &tileMap, glm::vec2 feet, float width)
    {
        float beneath = width * 0.5f - Settle;
        for (float across : {0.0f, -beneath, beneath})
        {
            glm::ivec2 under = tileMap.tileContaining(feet + glm::vec2(across, Settle));
            std::optional<AABB> ground = tileMap.groundAt(under);
            if (ground && std::abs(ground->top() - feet.y) <= Settle)
                return ground->top();
        }

        return feet.y;
    }

    constexpr float FarAway = 1.0e6f;

    float holdDurationOf(const AbilitiesData &abilitiesData, float holdFraction)
    {
        return abilitiesData.jump ? abilitiesData.jump->jumpDuration * holdFraction : 0.0f;
    }
}

JumpArc simulateJumpArc(const AbilitiesData &abilitiesData, float holdFraction)
{
    float holdDuration = holdDurationOf(abilitiesData, holdFraction);
    InputProgram inputs = aJumpHeldFor(holdDuration);
    Abilities abilities(abilitiesData);
    AbilityStates states;
    Observed observed;
    float elapsed = 0.0f;
    auto next = [&]
    {
        InputIntentions pressed = replaying(inputs, elapsed, 0.0f, FarAway);
        elapsed += PhysicsStep;
        return pressed;
    };

    observed.contacts.onGround = true;
    glm::vec2 takeOff = abilities.decide(PhysicsStep, next(), observed, states);
    if (takeOff.y >= 0.0f)
        return {};

    std::vector<glm::vec2> offsets{glm::vec2(0.0f)};
    glm::vec2 offset = takeOff * PhysicsStep;
    offsets.push_back(offset);

    observed.contacts.onGround = false;
    for (int step = 1; step < MaximumSteps; ++step)
    {
        offset += abilities.decide(PhysicsStep, next(), observed, states) * PhysicsStep;
        offsets.push_back(offset);

        if (offset.y >= 0.0f)
            return JumpArc{holdDuration, holdFraction, offsets};
    }

    return {};
}

std::vector<JumpArc> simulateJumpArcs(const AbilitiesData &abilitiesData)
{
    std::vector<JumpArc> arcs;

    for (float holdFraction : HoldFractions)
    {
        JumpArc arc = simulateJumpArc(abilitiesData, holdFraction);
        if (!arc.offsets.empty())
            arcs.push_back(arc);
    }

    return arcs;
}

JumpAttempt simulateJumpAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction,
    float holdFraction)
{
    return simulateInputsAgainst(
        tileMap,
        abilitiesData,
        physicsBodyData,
        takeOffFeet,
        aJumpHeldFor(holdDurationOf(abilitiesData, holdFraction)),
        takeOffFeet.x + direction * FarAway);
}

JumpAttempt simulateInputsAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    const InputProgram &inputs,
    float towardsX)
{
    Mover mover(abilitiesData, physicsBodyData);
    mover.standAt(takeOffFeet);
    mover.lookAround(tileMap);
    for (int settling = 0; settling < MaximumSettlingSteps && !mover.observed().contacts.onGround;
         ++settling)
        mover.step(PhysicsStep, InputIntentions{}, tileMap);

    JumpAttempt attempt;
    if (!feetSettledOn(mover.feet().y, takeOffFeet.y, physicsBodyData.stepHeight))
        return attempt;

    attempt.path.push_back(takeOffFeet);

    float elapsed = 0.0f;
    for (int step = 0; step < MaximumSteps; ++step)
    {
        mover.step(PhysicsStep, replaying(inputs, elapsed, mover.feet().x, towardsX), tileMap);
        elapsed += PhysicsStep;

        attempt.path.push_back(mover.feet());
        attempt.steps = step + 1;

        if (step > 0 && mover.observed().contacts.onGround)
        {
            attempt.path.back().y =
                restingOn(tileMap, attempt.path.back(), physicsBodyData.colliderSize.x);

            attempt.inputs = cutShortAt(inputs, elapsed);
            attempt.landed = true;
            return attempt;
        }
    }

    JumpAttempt capped;
    capped.steps = attempt.steps;
    capped.capped = true;
    return capped;
}
