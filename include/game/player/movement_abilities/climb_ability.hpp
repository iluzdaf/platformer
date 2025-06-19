#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;
class ClimbAbilityData;

class ClimbAbility : public MovementAbility
{
public:
    explicit ClimbAbility(const ClimbAbilityData &climbAbilityData);

    void fixedUpdate(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void update(MovementContext &movementContext, const PlayerState &playerState, float deltaTime) override;
    void tryClimb(MovementContext &movementContext, const PlayerState &playerState) override;
    void syncState(PlayerState &playerState) const override;
    void reset() override;

    float getClimbDuration() const;

private:
    float climbDuration = 1.0f,
          climbTime = 0.0f;
    bool climbRequested = false,
         climbing = false;
};