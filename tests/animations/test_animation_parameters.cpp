#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/animation_parameters.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

TEST_CASE("Standing on the ground, nothing much is happening", "[AnimationParameters]")
{
    Observed observed;
    observed.contacts.onGround = true;

    AnimationParameters parameters = parametersFrom(Decided{}, observed, false);

    AnimationParameters standing;
    standing.onGround = true;
    REQUIRE(parameters == standing);
}

TEST_CASE("Each parameter follows the one thing it watches", "[AnimationParameters]")
{
    Decided decided;
    Observed observed;

    observed.alive = false;
    REQUIRE_FALSE(parametersFrom(decided, observed, false).alive);
    observed.alive = true;

    decided.knockback.active = true;
    REQUIRE(parametersFrom(decided, observed, false).knockback);
    decided.knockback.active = false;

    decided.swing.phase = SwingPhase::Windup;
    REQUIRE(parametersFrom(decided, observed, false).swinging);
    decided.swing.phase = SwingPhase::Idle;

    decided.dash.active = true;
    REQUIRE(parametersFrom(decided, observed, false).dashing);
    decided.dash.active = false;

    observed.velocity = glm::vec2(0.0f, -40.0f);
    REQUIRE(parametersFrom(decided, observed, false).rising);
    REQUIRE_FALSE(parametersFrom(decided, observed, false).falling);

    observed.velocity = glm::vec2(0.0f, 40.0f);
    REQUIRE(parametersFrom(decided, observed, false).falling);
    REQUIRE_FALSE(parametersFrom(decided, observed, false).rising);

    observed.velocity = glm::vec2(0.2f, 0.0f);
    REQUIRE(parametersFrom(decided, observed, false).moving);
    observed.velocity = glm::vec2(0.05f, 0.0f);
    REQUIRE_FALSE(parametersFrom(decided, observed, false).moving);

    REQUIRE(parametersFrom(decided, observed, true).finished);
}

TEST_CASE(
    "Climbing is a hang that is moving, and a wall is a slide or a hang",
    "[AnimationParameters]")
{
    Decided decided;
    Observed observed;

    decided.wallSlide.active = true;
    REQUIRE(parametersFrom(decided, observed, false).onWall);
    REQUIRE_FALSE(parametersFrom(decided, observed, false).climbing);
    decided.wallSlide.active = false;

    decided.wallHang.active = true;
    REQUIRE(parametersFrom(decided, observed, false).onWall);
    REQUIRE_FALSE(parametersFrom(decided, observed, false).climbing);

    decided.wallClimb.velocity.y = -40.0f;
    REQUIRE(parametersFrom(decided, observed, false).climbing);

    decided.wallHang.active = false;
    decided.wallSlide.active = true;
    REQUIRE_FALSE(parametersFrom(decided, observed, false).climbing);
}

TEST_CASE("The parameters carry the state the machine is in", "[AnimationParameters]")
{
    REQUIRE(parametersFrom(Decided{}, Observed{}, false, "sleep").inState == "sleep");
    REQUIRE(parametersFrom(Decided{}, Observed{}, false).inState.empty());
}
