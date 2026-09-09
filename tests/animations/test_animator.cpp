#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>
#include "animations/animator.hpp"
#include "animations/frame_animation_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"
#include "actor/decided.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/observed.hpp"
#include "animations/animator_data.hpp"
#include "helpers/ladders.hpp"

namespace
{
    FrameAnimation animationOfFrame(int frame)
    {
        return FrameAnimation(FrameAnimationData({frame}, 1.0f));
    }

    Observed walkingOnGround()
    {
        Observed observed;
        observed.contacts.onGround = true;
        observed.velocity = glm::vec2(50.0f, 0.0f);
        return observed;
    }
}

TEST_CASE("Plays the animation for the state it is in", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("walk", animationOfFrame(2));

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.state() == "walk");
}

TEST_CASE("Falls back to idle for a state it has no animation for", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));

    REQUIRE_NOTHROW(animator.animate(0.01f, Decided{}, walkingOnGround()));
    REQUIRE(animator.state() == "idle");
}

TEST_CASE("An actor without airborne animations survives being airborne", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));

    Decided decided;

    Observed observed;
    observed.contacts.onGround = false;
    observed.velocity = glm::vec2(0.0f, 40.0f);

    REQUIRE_NOTHROW(animator.animate(0.01f, decided, observed));
    REQUIRE(animator.state() == "idle");

    decided.dash.active = true;
    REQUIRE_NOTHROW(animator.animate(0.01f, decided, observed));
    REQUIRE(animator.state() == "idle");
}

TEST_CASE("Off the ground, the observed velocity says whether it is a jump or a fall", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("jump", animationOfFrame(2));
    animator.add("fall", animationOfFrame(3));
    Observed airborne;
    airborne.contacts.onGround = false;

    airborne.velocity = glm::vec2(0.0f, -40.0f);
    animator.animate(0.01f, Decided{}, airborne);
    REQUIRE(animator.state() == "jump");

    airborne.velocity = glm::vec2(0.0f, 40.0f);
    animator.animate(0.01f, Decided{}, airborne);
    REQUIRE(animator.state() == "fall");
}

TEST_CASE("A swing shows the attack, and a corpse shows dead, over everything else", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("dash", animationOfFrame(2));
    animator.add("attack", animationOfFrame(3));
    animator.add("dead", animationOfFrame(4));
    Decided swingingWhileDashing;
    swingingWhileDashing.dash.active = true;
    swingingWhileDashing.swing.phase = SwingPhase::Windup;

    animator.animate(0.01f, swingingWhileDashing, walkingOnGround());
    REQUIRE(animator.state() == "attack");

    Observed dead = walkingOnGround();
    dead.alive = false;
    animator.animate(0.01f, swingingWhileDashing, dead);
    REQUIRE(animator.state() == "dead");
}

TEST_CASE("A knockback shows over a swing, a dash and the ground", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("dash", animationOfFrame(2));
    animator.add("attack", animationOfFrame(3));
    animator.add("knockback", animationOfFrame(5));
    Decided pushedWhileSwinging;
    pushedWhileSwinging.dash.active = true;
    pushedWhileSwinging.swing.phase = SwingPhase::Windup;
    pushedWhileSwinging.knockback.active = true;

    animator.animate(0.01f, pushedWhileSwinging, walkingOnGround());

    REQUIRE(animator.state() == "knockback");
}

TEST_CASE("A corpse shows dead even while it is still being pushed", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("knockback", animationOfFrame(5));
    animator.add("dead", animationOfFrame(4));
    Decided pushed;
    pushed.knockback.active = true;
    Observed dead = walkingOnGround();
    dead.alive = false;

    animator.animate(0.01f, pushed, dead);

    REQUIRE(animator.state() == "dead");
}

TEST_CASE("Hanging shows the climb only while it is moving", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("wallSlide", animationOfFrame(2));
    animator.add("climb", animationOfFrame(6));
    Observed offTheGround;
    Decided hanging;
    hanging.wallHang.active = true;

    animator.animate(0.01f, hanging, offTheGround);
    REQUIRE(animator.state() == "wallSlide");

    hanging.wallClimb.velocity.y = -40.0f;
    animator.animate(0.01f, hanging, offTheGround);
    REQUIRE(animator.state() == "climb");

    hanging.wallClimb.velocity.y = 40.0f;
    animator.animate(0.01f, hanging, offTheGround);
    REQUIRE(animator.state() == "climb");
}

TEST_CASE("A slide that is not a hang never shows the climb", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("wallSlide", animationOfFrame(2));
    animator.add("climb", animationOfFrame(6));
    Observed offTheGround;
    Decided sliding;
    sliding.wallSlide.active = true;
    sliding.wallClimb.velocity.y = -40.0f;

    animator.animate(0.01f, sliding, offTheGround);

    REQUIRE(animator.state() == "wallSlide");
}

TEST_CASE("Entering a state says its clip's opening cue", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("walk", FrameAnimation(FrameAnimationData{{2}, 1.0f, {{0, "onFootstep"}}}));

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.takeCues() == std::vector<std::string>{"onFootstep"});
}

TEST_CASE("Staying in a state does not repeat its opening cue", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    animator.add("walk", FrameAnimation(FrameAnimationData{{2}, 1.0f, {{0, "onFootstep"}}}));
    animator.animate(0.01f, Decided{}, walkingOnGround());
    animator.takeCues();

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.takeCues().empty());
}

TEST_CASE("The animator says when the clip it is playing has finished", "[Animator]")
{
    Animator animator(everyPictureLadder());
    animator.add("idle", animationOfFrame(1));
    FrameAnimationData once{{2, 3}, 0.1f};
    once.loops = false;
    animator.add("walk", FrameAnimation(once));

    animator.animate(0.01f, Decided{}, walkingOnGround());
    REQUIRE_FALSE(animator.finished());

    animator.animate(0.3f, Decided{}, walkingOnGround());
    REQUIRE(animator.finished());
}

TEST_CASE("A ladder given as data drives the animator", "[Animator]")
{
    AnimationWhen moving;
    moving.moving = true;
    AnimationWhen still;
    still.moving = false;
    AnimatorData ladder{{{"", "walk", moving}, {"walk", "idle", still}}};
    Animator animator(ladder);
    animator.add("idle", animationOfFrame(1));
    animator.add("walk", animationOfFrame(2));
    animator.add("jump", animationOfFrame(3));

    animator.animate(0.01f, Decided{}, walkingOnGround());
    REQUIRE(animator.state() == "walk");

    Observed airborne;
    airborne.velocity = glm::vec2(0.0f, -40.0f);
    animator.animate(0.01f, Decided{}, airborne);
    REQUIRE(animator.state() == "idle");

    animator.animate(0.01f, Decided{}, walkingOnGround());
    REQUIRE(animator.state() == "walk");
}

TEST_CASE("A rung with a from only fires from that state", "[Animator]")
{
    AnimationWhen always;
    AnimatorData ladder{{{"walk", "jump", always}}};
    Animator animator(ladder);
    animator.add("idle", animationOfFrame(1));
    animator.add("walk", animationOfFrame(2));
    animator.add("jump", animationOfFrame(3));

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.state() == "idle");
}

TEST_CASE("The animator can say the ladder it walks", "[Animator]")
{
    Animator animator(everyPictureLadder());

    REQUIRE(animator.ladder() == everyPictureLadder());
}
