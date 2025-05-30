#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"

DashAbility::DashAbility(const DashAbilityData &dashAbilityData)
    : dashSpeed(dashAbilityData.dashSpeed),
      dashDuration(dashAbilityData.dashDuration),
      dashCooldown(dashAbilityData.dashCooldown)
{
}

void DashAbility::update(MovementContext &context, float deltaTime)
{
    if (!canDash())
        dashCooldownLeft -= deltaTime;

    if (dashing())
    {
        glm::vec2 velocity = context.getVelocity();
        dashTimeLeft -= deltaTime;

        if (dashing())
        {
            velocity.x = dashSpeed * dashDirection;
        }

        velocity.y = 0.0f;
        context.setVelocity(velocity);
    }
}

void DashAbility::tryDash(MovementContext &context)
{
    if (canDash() && !dashing())
    {
        dashDirection = context.facingLeft() ? -1 : 1;
        dashTimeLeft = dashDuration;
        dashCooldownLeft = dashCooldown;
        glm::vec2 velocity = context.getVelocity();
        velocity.y = 0.0f;
        context.setVelocity(velocity);
    }
}

bool DashAbility::canDash() const
{
    return dashCooldownLeft <= 0.0f;
}

bool DashAbility::dashing() const
{
    return dashTimeLeft > 0.0f;
}

float DashAbility::getDashDuration() const
{
    return dashDuration;
}

float DashAbility::getDashCooldown() const
{
    return dashCooldown;
}