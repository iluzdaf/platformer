#pragma once

#include "game/player/movement_context.hpp"
#include "game/player/player_state.hpp"

class MockPlayer : public MovementContext
{
public:
    PlayerState playerState;
    glm::vec2 getPosition() const override { return playerState.position; }
    glm::vec2 getVelocity() const override { return playerState.velocity; }
    void setVelocity(const glm::vec2 &v) override { playerState.velocity = v; }
    bool onGround() const override { return playerState.onGround; }
    void setOnGround(bool isOnGround) override { playerState.onGround = isOnGround; };
    bool facingLeft() const override { return playerState.facingLeft; }
    void setFacingLeft(bool isFacingLeft) override { playerState.facingLeft = isFacingLeft; }
    const PlayerState &getPlayerState() const override { return playerState; }
    bool touchingRightWall() const override { return playerState.touchingRightWall; }
    bool touchingLeftWall() const override { return playerState.touchingLeftWall; }
    void setTouchingLeftWall(bool isTouchingLeftWall) { playerState.touchingLeftWall = isTouchingLeftWall; }
    void setTouchingRightWall(bool isTouchingRightWall) { playerState.touchingRightWall = isTouchingRightWall; }
};