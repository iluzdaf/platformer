#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class WallSlideAbilityData;

class WallSlideAbility : public MovementAbility
{
public:
    explicit WallSlideAbility(const WallSlideAbilityData &wallSlideAbilityData);

    void fixedUpdate(
        MovementContext &movementContext,
        const PlayerState &playerState,
        float deltaTime) override;
    void update(
        MovementContext &movementContext,
        const PlayerState &playerState,
        float deltaTime) override;
    void syncState(PlayerState &playerState) const override;
    void reset() override;
    void tryMoveLeft(
        MovementContext &movementContext,
        const PlayerState &playerState) override;
    void tryMoveRight(
        MovementContext &movementContext,
        const PlayerState &playerState) override;

    float getHangDuration() const;
    void resetHangTime();

private:
    bool wallSliding = false;
    float slideSpeed = 35.0f;
    float hangDuration = 0.2f;
    float hangTime = 0;
    bool moveLeftRequested = false;
    bool moveRightRequested = false;
};