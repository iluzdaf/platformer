#pragma once

struct WallJumpAbilityData
{
    float jumpSpeed = -280.0f,
          horizontalSpeed = 200.0f,
          wallJumpDuration = 0.25f,
          wallJumpBufferDuration = 0.1f;
    int maxJumpCount = 2;
};