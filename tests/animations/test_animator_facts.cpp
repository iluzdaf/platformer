#include <string>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    bool fact(
        const char *name,
        const Asked &asked,
        const Decided &decided,
        const Observed &observed,
        bool finished = false)
    {
        return rowNamed(animatorRows(), name)
            ->holds(asked, AnimatorFacts{decided, observed, finished, ""});
    }
}

TEST_CASE("Each fact follows the one thing it watches", "[AnimatorFacts]")
{
    Decided decided;
    Observed observed;

    REQUIRE(fact("alive", true, decided, observed));
    observed.alive = false;
    REQUIRE(fact("alive", false, decided, observed));
    observed.alive = true;

    decided.knockback.active = true;
    REQUIRE(fact("knockback", true, decided, observed));
    decided.knockback.active = false;

    decided.swing.phase = SwingPhase::Windup;
    REQUIRE(fact("swinging", true, decided, observed));
    decided.swing.phase = SwingPhase::Idle;

    decided.dash.active = true;
    REQUIRE(fact("dashing", true, decided, observed));
    decided.dash.active = false;

    observed.velocity = glm::vec2(0.0f, -40.0f);
    REQUIRE(fact("rising", true, decided, observed));
    REQUIRE(fact("falling", false, decided, observed));

    observed.velocity = glm::vec2(0.0f, 40.0f);
    REQUIRE(fact("falling", true, decided, observed));
    REQUIRE(fact("rising", false, decided, observed));

    observed.velocity = glm::vec2(0.2f, 0.0f);
    REQUIRE(fact("moving", true, decided, observed));
    observed.velocity = glm::vec2(0.05f, 0.0f);
    REQUIRE(fact("moving", false, decided, observed));

    REQUIRE(fact("finished", true, decided, observed, true));
    REQUIRE(fact("finished", false, decided, observed, false));
}

TEST_CASE("Climbing is a hang that is moving, and a wall is a slide or a hang", "[AnimatorFacts]")
{
    Decided decided;
    Observed observed;

    decided.wallSlide.active = true;
    REQUIRE(fact("onWall", true, decided, observed));
    REQUIRE(fact("climbing", false, decided, observed));
    decided.wallSlide.active = false;

    decided.wallHang.active = true;
    REQUIRE(fact("onWall", true, decided, observed));
    REQUIRE(fact("climbing", false, decided, observed));

    decided.wallClimb.velocity.y = -40.0f;
    REQUIRE(fact("climbing", true, decided, observed));

    decided.wallHang.active = false;
    decided.wallSlide.active = true;
    REQUIRE(fact("climbing", false, decided, observed));
}

TEST_CASE(
    "The animator's rows are the facts a rung may ask about, in the order they are worded",
    "[AnimatorFacts]")
{
    std::string names;
    for (const FactRow<AnimatorFacts> &row : animatorRows())
        names += std::string(row.name) + " ";

    REQUIRE(
        names == "alive knockback swinging dashing onGround climbing onWall rising falling moving "
                 "finished inState ");
    REQUIRE(rowNamed(animatorRows(), "inState")->kind == AskedKind::Name);
    REQUIRE(rowNamed(animatorRows(), "somersault") == nullptr);
}
