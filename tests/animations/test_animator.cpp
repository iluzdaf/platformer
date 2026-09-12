#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <string>
#include "animations/animator.hpp"
#include "animations/frame_animation_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "actor/decided.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/observed.hpp"
#include "animations/animation_ladder_data.hpp"
#include "conditions/asked.hpp"
#include "helpers/ladders.hpp"

namespace
{
    FrameAnimationData animationDataOfFrame(int frame)
    {
        return FrameAnimationData({frame}, 1.0f);
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
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = animationDataOfFrame(2);
    data.ladder = AnimationLadderData{{walkTransition(), idleTransition()}};
    Animator animator(data);

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.state() == "walk");
}

TEST_CASE("Falls back to idle for a state it has no animation for", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.ladder = AnimationLadderData{{idleTransition()}};
    Animator animator(data);

    REQUIRE_NOTHROW(animator.animate(0.01f, Decided{}, walkingOnGround()));
    REQUIRE(animator.state() == "idle");
}

TEST_CASE("An actor without airborne animations survives being airborne", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.ladder = AnimationLadderData{{idleTransition()}};
    Animator animator(data);

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
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["jump"] = animationDataOfFrame(2);
    data.clips["fall"] = animationDataOfFrame(3);
    data.ladder = AnimationLadderData{{jumpTransition(), fallTransition(), idleTransition()}};
    Animator animator(data);
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
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["dash"] = animationDataOfFrame(2);
    data.clips["attack"] = animationDataOfFrame(3);
    data.clips["dead"] = animationDataOfFrame(4);
    data.ladder = AnimationLadderData{
        {deadTransition(), swingTransition(), dashTransition(), idleTransition()}};
    Animator animator(data);
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
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["dash"] = animationDataOfFrame(2);
    data.clips["attack"] = animationDataOfFrame(3);
    data.clips["knockback"] = animationDataOfFrame(5);
    data.ladder = AnimationLadderData{
        {knockbackTransition(), swingTransition(), dashTransition(), idleTransition()}};
    Animator animator(data);
    Decided pushedWhileSwinging;
    pushedWhileSwinging.dash.active = true;
    pushedWhileSwinging.swing.phase = SwingPhase::Windup;
    pushedWhileSwinging.knockback.active = true;

    animator.animate(0.01f, pushedWhileSwinging, walkingOnGround());

    REQUIRE(animator.state() == "knockback");
}

TEST_CASE("A corpse shows dead even while it is still being pushed", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["knockback"] = animationDataOfFrame(5);
    data.clips["dead"] = animationDataOfFrame(4);
    data.ladder = AnimationLadderData{{deadTransition(), knockbackTransition(), idleTransition()}};
    Animator animator(data);
    Decided pushed;
    pushed.knockback.active = true;
    Observed dead = walkingOnGround();
    dead.alive = false;

    animator.animate(0.01f, pushed, dead);

    REQUIRE(animator.state() == "dead");
}

TEST_CASE("Hanging shows the climb only while it is moving", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["wallSlide"] = animationDataOfFrame(2);
    data.clips["climb"] = animationDataOfFrame(6);
    data.ladder = AnimationLadderData{{
        climbTransition(),
        wallSlideTransition(),
        idleTransition(),
    }};
    Animator animator(data);
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
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["wallSlide"] = animationDataOfFrame(2);
    data.clips["climb"] = animationDataOfFrame(6);
    data.ladder = AnimationLadderData{{climbTransition(), wallSlideTransition(), idleTransition()}};
    Animator animator(data);
    Observed offTheGround;
    Decided sliding;
    sliding.wallSlide.active = true;
    sliding.wallClimb.velocity.y = -40.0f;

    animator.animate(0.01f, sliding, offTheGround);

    REQUIRE(animator.state() == "wallSlide");
}

TEST_CASE("Entering a state says its clip's opening cue", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = FrameAnimationData({2}, 1.0f, {FrameCueData{0, "onFootstep"}});
    data.ladder = AnimationLadderData{{walkTransition(), idleTransition()}};
    Animator animator(data);

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.takeCues() == std::vector<std::string>{"onFootstep"});
}

TEST_CASE("Staying in a state does not repeat its opening cue", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = FrameAnimationData({2}, 1.0f, {FrameCueData{0, "onFootstep"}});
    data.ladder = AnimationLadderData{{walkTransition(), idleTransition()}};
    Animator animator(data);
    animator.animate(0.01f, Decided{}, walkingOnGround());
    animator.takeCues();

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.takeCues().empty());
}

TEST_CASE("The animator says when the clip it is playing has finished", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = FrameAnimationData({2, 3}, 0.1f, {}, false);
    data.ladder = AnimationLadderData{{walkTransition(), idleTransition()}};
    Animator animator(data);
    data.clips["walk"].loops = false;

    animator.animate(0.01f, Decided{}, walkingOnGround());
    REQUIRE_FALSE(animator.finished());

    animator.animate(0.3f, Decided{}, walkingOnGround());
    REQUIRE(animator.finished());
}

TEST_CASE("A ladder given as data drives the animator", "[Animator]")
{
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = animationDataOfFrame(2);
    AnimationWhenData moving;
    moving["moving"] = true;
    AnimationWhenData notMoving;
    notMoving["moving"] = false;
    data.ladder = AnimationLadderData{{fromAnyTo("walk", moving), {"walk", "idle", notMoving}}};
    Animator animator(data);

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
    AnimationWhenData always;
    AnimationLadderData ladder{{{"walk", "jump", always}}};
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["walk"] = animationDataOfFrame(2);
    data.clips["jump"] = animationDataOfFrame(3);
    data.ladder = ladder;
    Animator animator(data);

    animator.animate(0.01f, Decided{}, walkingOnGround());

    REQUIRE(animator.state() == "idle");
}

TEST_CASE("A rung may ask which state the machine is in", "[Animator]")
{
    AnimationWhenData asleep;
    asleep["inState"] = std::string("sleep");
    AnimationWhenData otherwise;
    otherwise["onGround"] = true;
    AnimationLadderData ladder{{{"", "sleep", asleep}, {"", "idle", otherwise}}};
    AnimatorData data;
    data.startClip = "idle";
    data.clips["idle"] = animationDataOfFrame(1);
    data.clips["sleep"] = animationDataOfFrame(2);
    data.ladder = ladder;
    Animator animator(data);

    animator.animate(0.01f, Decided{}, walkingOnGround(), "sleep");
    REQUIRE(animator.state() == "sleep");

    animator.animate(0.01f, Decided{}, walkingOnGround(), "charge");
    REQUIRE(animator.state() == "idle");

    animator.animate(0.01f, Decided{}, walkingOnGround());
    REQUIRE(animator.state() == "idle");
}
