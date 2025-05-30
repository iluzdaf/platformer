#pragma once
#include "game/player/movement_abilities/movement_ability.hpp"
class MovementContext;
class JumpAbilityData;

class JumpAbility : public MovementAbility
{
public:
    explicit JumpAbility(const JumpAbilityData &jumpAbilityData);
    void update(MovementContext &context, float deltaTime) override;
    void tryJump(MovementContext &context) override;
    void resetJumps();
    int getMaxJumpCount() const;
    float getJumpSpeed() const;

private:
    int maxJumpCount = 2;
    int jumpCount = 0;
    float jumpSpeed = -280;
};