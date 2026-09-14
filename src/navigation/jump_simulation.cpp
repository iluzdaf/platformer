#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include "navigation/jump_simulation.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/move_ability_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/actor_contact_state.hpp"
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

    float restingOn(const TileMap &tileMap, glm::vec2 feet, const PhysicsBodyData &body)
    {
        std::optional<float> underTheFeet;
        for (float across : {-Settle, Settle})
        {
            glm::ivec2 under = tileMap.tileContaining(feet + glm::vec2(across, Settle));
            std::optional<AABB> ground = tileMap.groundAt(under);
            if (ground && ground->top() >= feet.y - Settle &&
                ground->top() <= feet.y + body.stepHeight + Settle)
                underTheFeet = std::max(underTheFeet.value_or(ground->top()), ground->top());
        }
        if (underTheFeet)
            return *underTheFeet;

        float beneath = body.colliderSize.x * 0.5f - Settle;
        for (float across : {-beneath, beneath})
        {
            glm::ivec2 under = tileMap.tileContaining(feet + glm::vec2(across, Settle));
            std::optional<AABB> ground = tileMap.groundAt(under);
            if (ground && std::abs(ground->top() - feet.y) <= Settle)
                return ground->top();
        }

        return feet.y;
    }

    constexpr float FarAway = 1.0e6f;

    float strideOf(const AbilitiesData &abilitiesData)
    {
        return abilitiesData.move ? abilitiesData.move->moveSpeed * PhysicsStep : 0.0f;
    }
    constexpr float StillOnTheGroundFor = 0.1f;

    bool settleOnto(
        Mover &mover,
        const TileMap &tileMap,
        glm::vec2 takeOffFeet,
        const PhysicsBodyData &physicsBodyData)
    {
        mover.standAt(takeOffFeet);
        mover.lookAround(tileMap);
        for (int settling = 0;
             settling < MaximumSettlingSteps && !mover.observed().contacts.onGround;
             ++settling)
            mover.step(PhysicsStep, InputIntentions{}, tileMap);

        return feetSettledOn(mover.feet().y, takeOffFeet.y, physicsBodyData.stepHeight);
    }

    bool takeHold(Mover &mover, const TileMap &tileMap, glm::vec2 takeOffFeet, float wallDirection)
    {
        mover.standAt(takeOffFeet);
        mover.lookAround(tileMap);
        InputIntentions holding;
        holding.climbRequested = true;
        holding.direction.x = wallDirection;
        for (int settling = 0; settling < MaximumSettlingSteps && !mover.states().wallHang.active;
             ++settling)
            mover.step(PhysicsStep, holding, tileMap);

        return mover.states().wallHang.active &&
               std::abs(mover.feet().y - takeOffFeet.y) <= ClimbArrivesWithin;
    }

    bool getReady(
        Mover &mover,
        const TileMap &tileMap,
        glm::vec2 takeOffFeet,
        float wallDirection,
        const PhysicsBodyData &physicsBodyData)
    {
        if (wallDirection == 0.0f)
            return settleOnto(mover, tileMap, takeOffFeet, physicsBodyData);

        return takeHold(mover, tileMap, takeOffFeet, wallDirection);
    }

    void comeToRest(JumpAttempt &attempt, const TileMap &tileMap, const PhysicsBodyData &body)
    {
        attempt.path.back().y = restingOn(tileMap, attempt.path.back(), body);
        attempt.landed = true;
    }

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

JumpAttempt simulateWallJumpAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float wallDirection)
{
    return simulateInputsAgainst(
        tileMap,
        abilitiesData,
        physicsBodyData,
        takeOffFeet,
        aWallJumpAwayFrom(wallDirection),
        takeOffFeet.x - wallDirection * FarAway,
        wallDirection);
}

JumpAttempt simulateInputsAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    const InputProgram &inputs,
    float towardsX,
    float wallDirection)
{
    Mover mover(abilitiesData, physicsBodyData);
    JumpAttempt attempt;
    if (!getReady(mover, tileMap, takeOffFeet, wallDirection, physicsBodyData))
        return attempt;

    attempt.path.push_back(takeOffFeet);

    float elapsed = 0.0f;
    bool airborne = false;
    bool leftTheWall = false;
    for (int step = 0; step < MaximumSteps; ++step)
    {
        mover.step(
            PhysicsStep,
            replaying(inputs, elapsed, mover.feet().x, towardsX, strideOf(abilitiesData)),
            tileMap);
        elapsed += PhysicsStep;

        attempt.path.push_back(mover.feet());
        attempt.steps = step + 1;

        const ActorContactState &contacts = mover.observed().contacts;
        bool atTheWall =
            wallDirection != 0.0f &&
            (wallDirection < 0.0f ? contacts.touchingLeftWall : contacts.touchingRightWall);
        if (atTheWall && leftTheWall)
        {
            attempt.cameBackToTheWall = true;
            return attempt;
        }
        leftTheWall = leftTheWall || !atTheWall;

        if (!contacts.onGround)
            airborne = true;
        else if (airborne)
        {
            attempt.inputs = cutShortAt(inputs, elapsed);
            comeToRest(attempt, tileMap, physicsBodyData);
            return attempt;
        }
        else if (elapsed > durationOf(inputs) + StillOnTheGroundFor)
            return attempt;
    }

    JumpAttempt capped;
    capped.steps = attempt.steps;
    capped.capped = true;
    return capped;
}

JumpAttempt simulateFallAgainst(
    const TileMap &tileMap,
    const AbilitiesData &abilitiesData,
    const PhysicsBodyData &physicsBodyData,
    glm::vec2 takeOffFeet,
    float direction)
{
    Mover mover(abilitiesData, physicsBodyData);
    JumpAttempt attempt;
    if (!settleOnto(mover, tileMap, takeOffFeet, physicsBodyData))
        return attempt;

    attempt.path.push_back(takeOffFeet);

    InputIntentions walkingOff;
    walkingOff.direction.x = direction;
    float elapsed = 0.0f;
    std::optional<float> leftTheGroundAt;
    for (int step = 0; step < MaximumSteps; ++step)
    {
        float wasAtX = mover.feet().x;
        mover.step(PhysicsStep, leftTheGroundAt ? InputIntentions{} : walkingOff, tileMap);
        elapsed += PhysicsStep;

        attempt.path.push_back(mover.feet());
        attempt.steps = step + 1;

        bool onGround = mover.observed().contacts.onGround;
        if (!leftTheGroundAt && !onGround)
            leftTheGroundAt = elapsed;
        else if (leftTheGroundAt && onGround)
        {
            attempt.inputs = {{*leftTheGroundAt, walkingOff}};
            comeToRest(attempt, tileMap, physicsBodyData);
            return attempt;
        }
        else if (
            !leftTheGroundAt &&
            (mover.feet().x == wasAtX ||
             std::abs(mover.feet().x - takeOffFeet.x) > physicsBodyData.colliderSize.x))
            return attempt;
    }

    JumpAttempt capped;
    capped.steps = attempt.steps;
    capped.capped = true;
    return capped;
}
