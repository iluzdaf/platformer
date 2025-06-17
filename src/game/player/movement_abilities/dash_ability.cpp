#include "game/player/movement_abilities/dash_ability.hpp"
#include "game/player/movement_context.hpp"
#include "game/player/movement_abilities/dash_ability_data.hpp"
#include "game/player/player_state.hpp"

DashAbility::DashAbility(const DashAbilityData &dashAbilityData)
    : dashSpeed(dashAbilityData.dashSpeed),
      dashDuration(dashAbilityData.dashDuration),
      dashCooldown(dashAbilityData.dashCooldown)
{
    if (dashSpeed <= 0)
        throw std::invalid_argument("dashSpeed must be greater than 0");
    if (dashDuration <= 0)
        throw std::invalid_argument("dashDuration must be greater than 0");
    if (dashCooldown <= 0)
        throw std::invalid_argument("dashCooldown must be greater than 0");
    if (dashCooldown <= dashDuration)
        throw std::invalid_argument("dashCooldown must be greater than dashDuration");
}

void DashAbility::fixedUpdate(
    MovementContext &movementContext,
    const PlayerState & playerState,
    float deltaTime)
{
    if (dashCooldownLeft > 0.0f)
        dashCooldownLeft -= deltaTime;

    if (!dashing())
        return;

    if (playerState.touchingLeftWall || playerState.touchingRightWall)
    {
        dashTimeLeft = 0.0f;
        return;
    }

    dashTimeLeft -= deltaTime;

    glm::vec2 velocity = movementContext.getVelocity();
    velocity.x = dashSpeed * dashDirection;
    velocity.y = 0.0f;
    movementContext.setVelocity(velocity);
}

void DashAbility::update(
    MovementContext & /*movementContext*/,
    const PlayerState & /*playerState*/,
    float /*deltaTime*/)
{
}

void DashAbility::tryDash(
    MovementContext &movementContext,
    const PlayerState &playerState)
{
    if (playerState.wallSliding || playerState.wallJumping || dashing() || !canDash())
        return;

    dashDirection = playerState.facingLeft ? -1 : 1;
    dashTimeLeft = dashDuration;
    dashCooldownLeft = dashCooldown;
    glm::vec2 velocity = movementContext.getVelocity();
    velocity.y = 0.0f;
    movementContext.setVelocity(velocity);
    movementContext.emitDash();
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

void DashAbility::syncState(PlayerState &playerState) const
{
    playerState.dashDuration = dashDuration;
    playerState.dashing = dashing();
}

void DashAbility::reset()
{
    dashTimeLeft = 0;
    dashCooldownLeft = 0;
    dashDirection = 1;
}