#include <catch2/catch_test_macros.hpp>
#include <string>
#include "actor/actor_animation_state.hpp"

TEST_CASE("Every animation state says its name", "[ActorAnimationState]")
{
    REQUIRE(std::string(toString(ActorAnimationState::Idle)) == "Idle");
    REQUIRE(std::string(toString(ActorAnimationState::Walk)) == "Walk");
    REQUIRE(std::string(toString(ActorAnimationState::Jump)) == "Jump");
    REQUIRE(std::string(toString(ActorAnimationState::Fall)) == "Fall");
    REQUIRE(std::string(toString(ActorAnimationState::WallSlide)) == "WallSlide");
    REQUIRE(std::string(toString(ActorAnimationState::Dash)) == "Dash");
}

TEST_CASE("A state that is not one of them says so", "[ActorAnimationState]")
{
    REQUIRE(std::string(toString(static_cast<ActorAnimationState>(99))) == "Unknown");
}
