#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <map>
#include <concepts>
#include <string_view>
#include <utility>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    bool fact(
        const char *name,
        const Asked &asked,
        const AbilityStates &states,
        const Observed &observed,
        bool finished = false)
    {
        return rowNamed(animatorRows(), name)
            ->holds(asked, AnimatorFacts{states, observed, finished, ""});
    }
}

TEST_CASE("Each fact follows the one thing it watches", "[AnimatorFacts]")
{
    AbilityStates states;
    Observed observed;

    REQUIRE(fact("alive", true, states, observed));
    observed.alive = false;
    REQUIRE(fact("alive", false, states, observed));
    observed.alive = true;

    states.knockback.active = true;
    REQUIRE(fact("knockback", true, states, observed));
    states.knockback.active = false;

    states.swing.phase = SwingPhase::Windup;
    REQUIRE(fact("swinging", true, states, observed));
    states.swing.phase = SwingPhase::Idle;

    states.dash.active = true;
    REQUIRE(fact("dashing", true, states, observed));
    states.dash.active = false;

    states.pounce.active = true;
    REQUIRE(fact("pouncing", true, states, observed));
    REQUIRE(fact("dashing", false, states, observed));
    states.pounce.active = false;

    observed.velocity = glm::vec2(0.0f, -40.0f);
    REQUIRE(fact("rising", true, states, observed));
    REQUIRE(fact("falling", false, states, observed));

    observed.velocity = glm::vec2(0.0f, 40.0f);
    REQUIRE(fact("falling", true, states, observed));
    REQUIRE(fact("rising", false, states, observed));

    observed.velocity = glm::vec2(0.2f, 0.0f);
    REQUIRE(fact("moving", true, states, observed));
    observed.velocity = glm::vec2(0.05f, 0.0f);
    REQUIRE(fact("moving", false, states, observed));

    REQUIRE(fact("finished", true, states, observed, true));
    REQUIRE(fact("finished", false, states, observed, false));
}

TEST_CASE("Climbing is a hang that is moving, and a wall is a slide or a hang", "[AnimatorFacts]")
{
    AbilityStates states;
    Observed observed;

    states.wallSlide.active = true;
    REQUIRE(fact("onWall", true, states, observed));
    REQUIRE(fact("climbing", false, states, observed));
    states.wallSlide.active = false;

    states.wallHang.active = true;
    REQUIRE(fact("onWall", true, states, observed));
    REQUIRE(fact("climbing", false, states, observed));

    states.wallClimb.velocity.y = -40.0f;
    REQUIRE(fact("climbing", true, states, observed));

    states.wallHang.active = false;
    states.wallSlide.active = true;
    REQUIRE(fact("climbing", false, states, observed));
}

TEST_CASE(
    "The animator's rows are the facts a rule may ask about, in the order they are worded",
    "[AnimatorFacts]")
{
    std::string names;
    for (const FactRow<AnimatorFacts> &row : animatorRows())
        names += std::string(row.name) + " ";

    REQUIRE(
        names == "alive knockback swinging dashing pouncing charging onGround climbing onWall "
                 "rising falling "
                 "moving finished inState ");
    REQUIRE(rowNamed(animatorRows(), "inState")->kind == AskedKind::Name);
    REQUIRE(rowNamed(animatorRows(), "somersault") == nullptr);
}

namespace
{
    template <class T>
    concept CanBeActive = requires(const T &state) {
        { state.active } -> std::convertible_to<bool>;
    };

    template <std::size_t... I>
    std::vector<std::string_view> abilitiesThatCanBeActive(std::index_sequence<I...>)
    {
        std::vector<std::string_view> names;
        (
            [&]
            {
                if constexpr (CanBeActive<typename glz::reflect<AbilityStates>::template type<I>>)
                    names.push_back(glz::reflect<AbilityStates>::keys[I]);
            }(),
            ...);
        return names;
    }
}

TEST_CASE("Every ability that can be active is a fact, or says why it is not", "[AnimatorFacts]")
{
    const std::map<std::string_view, std::string_view> notAFactBecause{
        {"jump", "rising already says it; a jump is not a picture of its own"},
        {"wallJump", "it is a jump"},
        {"mantle", "there is no mantle picture yet; add a row when there is"},
        {"bite", "a bite is always ready, so it could never choose a picture"}};

    std::vector<std::string_view> flagged =
        abilitiesThatCanBeActive(std::make_index_sequence<glz::reflect<AbilityStates>::size>{});
    REQUIRE_FALSE(flagged.empty());

    for (std::string_view ability : flagged)
    {
        bool covered = false;
        for (const FactRow<AnimatorFacts> &row : animatorRows())
            covered = covered || row.covers == ability;

        INFO("ability \"" << ability << "\" can be active, and no row watches it nor says why not");
        REQUIRE((covered || notAFactBecause.contains(ability)));
    }
}

TEST_CASE("A row that says it watches a flag really reads it", "[AnimatorFacts]")
{
    for (const FactRow<AnimatorFacts> &row : animatorRows())
    {
        if (row.covers.empty())
            continue;

        AbilityStates off;
        AbilityStates on;
        if (row.covers == "dash")
            on.dash.active = true;
        else if (row.covers == "knockback")
            on.knockback.active = true;
        else if (row.covers == "pounce")
            on.pounce.active = true;
        else if (row.covers == "charge")
            on.charge.active = true;
        else if (row.covers == "wallSlide")
            on.wallSlide.active = true;
        else if (row.covers == "wallHang")
        {
            on.wallHang.active = true;
            on.wallClimb.velocity.y = -1.0f;
        }
        else
            FAIL(
                "row \"" << row.name << "\" covers \"" << row.covers
                         << "\", which this test does not know how to switch on");

        Observed observed;
        INFO("row \"" << row.name << "\"");
        REQUIRE(row.holds(true, AnimatorFacts{on, observed, false, ""}));
        REQUIRE_FALSE(row.holds(true, AnimatorFacts{off, observed, false, ""}));
    }
}
