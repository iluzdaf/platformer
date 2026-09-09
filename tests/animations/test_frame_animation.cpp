#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <utility>
#include <string>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"

TEST_CASE("Default Constucted FrameAnimation behaves correctly", "[FrameAnimation]")
{
    FrameAnimation frameAnimation;
    REQUIRE(frameAnimation.frame() == 0);

    frameAnimation.update(0.5f);
    REQUIRE(frameAnimation.frame() == 0);

    frameAnimation.reset();
    REQUIRE(frameAnimation.frame() == 0);
}

TEST_CASE("FrameAnimation updates frame based on time", "[FrameAnimation]")
{
    FrameAnimation frameAnimation(FrameAnimationData{{1, 2, 3}, 0.5f});

    SECTION("Starts at first frame")
    {
        REQUIRE(frameAnimation.frame() == 1);
    }

    SECTION("Advances to next frame after time")
    {
        frameAnimation.update(0.5f);
        REQUIRE(frameAnimation.frame() == 2);
    }

    SECTION("Wraps around after all frames")
    {
        frameAnimation.update(1.5f);
        REQUIRE(frameAnimation.frame() == 1);
    }

    SECTION("Multiple small steps accumulate")
    {
        frameAnimation.update(0.2f);
        frameAnimation.update(0.2f);
        frameAnimation.update(0.2f);
        REQUIRE(frameAnimation.frame() == 2);
    }

    SECTION("Reset returns to frame 0")
    {
        frameAnimation.update(1.0f);
        frameAnimation.reset();
        REQUIRE(frameAnimation.frame() == 1);
    }

    SECTION("No time passing leaves the frame where it was")
    {
        frameAnimation.update(0.0f);
        REQUIRE(frameAnimation.frame() == 1);
    }
}

TEST_CASE("An animation with no duration stays on its first frame", "[FrameAnimation]")
{
    FrameAnimation frameAnimation(FrameAnimationData{{1, 2, 3}, 0.0f});

    frameAnimation.update(0.5f);

    REQUIRE(frameAnimation.frame() == 1);
}
namespace
{
    FrameAnimationData aClipCueing(std::vector<FrameCueData> cues, int frameCount = 3)
    {
        std::vector<int> frames;
        for (int frame = 0; frame < frameCount; ++frame)
            frames.push_back(frame);
        return FrameAnimationData{frames, 0.1f, std::move(cues)};
    }
}

TEST_CASE("A clip says its opening cue as soon as it starts", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{0, "onStart"}}));

    REQUIRE(animation.takeCues() == std::vector<std::string>{"onStart"});
}

TEST_CASE("A cue is said on entering its frame, not before", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{1, "onSwing"}}));
    animation.takeCues();

    animation.update(0.05f);
    REQUIRE(animation.takeCues().empty());

    animation.update(0.05f);
    REQUIRE(animation.takeCues() == std::vector<std::string>{"onSwing"});
}

TEST_CASE("Crossing several frames in one step says each cue in order", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{1, "one"}, {2, "two"}}));
    animation.takeCues();

    animation.update(0.25f);

    REQUIRE(animation.takeCues() == std::vector<std::string>{"one", "two"});
}

TEST_CASE("A looping clip says its opening cue again each time round", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{0, "onFootstep"}}, 2));
    animation.takeCues();

    animation.update(0.2f);

    REQUIRE(animation.takeCues() == std::vector<std::string>{"onFootstep"});
}

TEST_CASE("A cue once taken is not said again", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{0, "onStart"}}));
    animation.takeCues();

    REQUIRE(animation.takeCues().empty());
}

TEST_CASE("Resetting a clip says its opening cue again", "[FrameAnimation]")
{
    FrameAnimation animation(aClipCueing({{0, "onStart"}}));
    animation.takeCues();

    animation.reset();

    REQUIRE(animation.takeCues() == std::vector<std::string>{"onStart"});
}

TEST_CASE("A clip with no frames has no cue to say", "[FrameAnimation]")
{
    FrameAnimation animation(FrameAnimationData{{}, 0.1f, {{0, "onStart"}}});

    animation.update(1.0f);

    REQUIRE(animation.takeCues().empty());
}
