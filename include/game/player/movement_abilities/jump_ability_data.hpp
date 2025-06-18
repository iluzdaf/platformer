#pragma once

struct JumpAbilityData
{
    float jumpSpeed = -320.0f,
          jumpBufferDuration = 0.1f,
          jumpCoyoteDuration = 0.2f;
    int maxJumpCount = 2;
};