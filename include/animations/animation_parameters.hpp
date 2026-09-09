#pragma once

struct Decided;
struct Observed;

struct AnimationParameters
{
    bool alive = true;
    bool knockback = false;
    bool swinging = false;
    bool dashing = false;
    bool onGround = false;
    bool climbing = false;
    bool onWall = false;
    bool rising = false;
    bool falling = false;
    bool moving = false;
    bool finished = false;

    bool operator==(const AnimationParameters &) const = default;
};

AnimationParameters parametersFrom(const Decided &decided, const Observed &observed, bool finished);
