#include <cmath>
#include <vector>
#include "navigation/jump_simulation.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/mover.hpp"
#include "input/input_intentions.hpp"
#include "navigation/jump_arc.hpp"
#include "physics/physics_body_data.hpp"
#include "tile_map/tile_map.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr int MaximumSteps = 1000;
    constexpr int MaximumSettlingSteps = 30;
    constexpr float HoldFractions[] = {1.0f, 0.75f, 0.5f, 0.25f};

    float holdDurationOf(const AbilitiesData &abilitiesData, float holdFraction)
    {
        return abilitiesData.jump ? abilitiesData.jump->jumpDuration * holdFraction : 0.0f;
    }

    class HoldingJump
    {
    public:
        HoldingJump(float direction, float holdFor) : direction(direction), holdFor(holdFor)
        {
        }

        InputIntentions next(float deltaTime)
        {
            InputIntentions inputIntentions;
            inputIntentions.direction.x = direction;
            if (heldFor < holdFor)
            {
                inputIntentions.jumpRequested = true;
                inputIntentions.jumpHeld = true;
                heldFor += deltaTime;
            }
            return inputIntentions;
        }

    private:
        float direction = 0.0f;
        float holdFor = 0.0f;
        float heldFor = 0.0f;
    };
}

JumpArc simulateJumpArc(const AbilitiesData &abilitiesData, float holdFraction)
{
    float holdDuration = holdDurationOf(abilitiesData, holdFraction);
    Abilities abilities(abilitiesData);
    AbilityStates states;
    Observed observed;
    HoldingJump holding(1.0f, holdDuration);

    observed.contacts.onGround = true;
    glm::vec2 takeOff = abilities.decide(PhysicsStep, holding.next(PhysicsStep), observed, states);
    if (takeOff.y >= 0.0f)
        return {};

    std::vector<glm::vec2> offsets{glm::vec2(0.0f)};
    glm::vec2 offset = takeOff * PhysicsStep;
    offsets.push_back(offset);

    observed.contacts.onGround = false;
    for (int step = 1; step < MaximumSteps; ++step)
    {
        offset += abilities.decide(PhysicsStep, holding.next(PhysicsStep), observed, states) *
                  PhysicsStep;
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
    Mover mover(abilitiesData, physicsBodyData);
    mover.standAt(takeOffFeet);
    mover.lookAround(tileMap);
    for (int settling = 0; settling < MaximumSettlingSteps && !mover.observed().contacts.onGround;
         ++settling)
        mover.step(PhysicsStep, InputIntentions{}, tileMap);

    HoldingJump holding(direction, holdDurationOf(abilitiesData, holdFraction));
    JumpAttempt attempt;
    attempt.path.push_back(takeOffFeet);

    for (int step = 0; step < MaximumSteps; ++step)
    {
        mover.step(PhysicsStep, holding.next(PhysicsStep), tileMap);

        attempt.path.push_back(mover.feet());
        attempt.steps = step + 1;

        if (step > 0 && mover.observed().contacts.onGround)
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
