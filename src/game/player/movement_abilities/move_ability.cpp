#include "game/player/movement_abilities/move_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/movement_abilities/move_ability_data.hpp"

MoveAbility::MoveAbility(const MoveAbilityData &moveAbilityData) : moveSpeed(moveAbilityData.moveSpeed)
{
}

void MoveAbility::update(MovementContext &context, float /*deltaTime*/)
{
    if (moveLeftRequested && !moveRightRequested)
    {
        glm::vec2 vel = context.getVelocity();
        vel.x = -moveSpeed;
        context.setVelocity(vel);
        context.setFacingLeft(true);
    }
    else if (moveRightRequested && !moveLeftRequested)
    {
        glm::vec2 vel = context.getVelocity();
        vel.x = moveSpeed;
        context.setVelocity(vel);
        context.setFacingLeft(false);
    }

    moveLeftRequested = false;
    moveRightRequested = false;
}

void MoveAbility::tryMoveLeft(MovementContext &context)
{
    moveLeftRequested = true;
}

void MoveAbility::tryMoveRight(MovementContext &context)
{
    moveRightRequested = true;
}

float MoveAbility::getMoveSpeed() const
{
    return moveSpeed;
}
