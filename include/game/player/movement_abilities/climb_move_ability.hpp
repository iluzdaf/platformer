#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;
class ClimbMoveAbilityData;

class ClimbMoveAbility : public MovementAbility
{
public:
    explicit ClimbMoveAbility(const ClimbMoveAbilityData &climbMoveAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryAscend(MovementContext &movementContext, const PlayerState &playerState) override;
    void tryDescend(MovementContext &movementContext, const PlayerState &playerState) override;
    void reset() override;

    float getClimbSpeed() const;

private:
    float climbSpeed = 80.0f;
    bool ascendRequested = false,
         descendRequested = false;
};