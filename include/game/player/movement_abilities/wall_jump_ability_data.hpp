#pragma once

struct WallJumpAbilityData
{
    float wallJumpSpeed = -280.0f,
          wallJumpHorizontalSpeed = 200.0f,
          wallJumpDuration = 0.25f,
          wallJumpBufferDuration = 0.1f,
          wallJumpCoyoteDuration = 0.1f;
    int maxWallJumpCount = 2;
};