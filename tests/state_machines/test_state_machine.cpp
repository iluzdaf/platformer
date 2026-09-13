#include <array>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"
#include "state_machines/state_machine.hpp"
#include "state_machines/state_machine_data.hpp"

namespace
{
    struct Lamp
    {
        bool switchedOn = false;
    };

    constexpr std::array LampRows{FactRow<Lamp>{
        "switchedOn",
        AskedKind::YesOrNo,
        "switched on",
        "switched off",
        [](const Asked &asked, const Lamp &lamp)
        { return std::get<bool>(asked) == lamp.switchedOn; },
        ""}};

    const Lamp On{true};
    const Lamp Off{false};
    const FactsData NothingDeclared;

    using LampData = StateMachineData<std::monostate>;

    StateData<std::monostate> aState(const std::string &name, float cooldown = 0.0f)
    {
        return {name, {}, cooldown};
    }

    TransitionData aTransition(
        const std::string &from,
        const std::string &to,
        const std::string &asking,
        const Asked &asked,
        float after = 0.0f)
    {
        TransitionData transition{from, to, {}, after};
        transition.when[asking] = asked;
        return transition;
    }

    LampData aLamp(float warmsUpFor = 0.0f, float restsFor = 0.0f)
    {
        return {
            {aState("dark"), aState("lit", restsFor)},
            {aTransition("dark", "lit", "switchedOn", true, warmsUpFor),
             aTransition("lit", "dark", "switchedOn", false)}};
    }

    void hold(StateMachine<Lamp> &machine, const Lamp &lamp, int ticks)
    {
        for (int tick = 0; tick < ticks; ++tick)
            machine.advance(0.01f, lamp, NothingDeclared);
    }
}

TEST_CASE("Starts in the first state it was given", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(), LampRows, NothingDeclared);

    REQUIRE(machine.activeName() == "dark");
    REQUIRE(machine.active() == 0);
}

TEST_CASE("Enters a state once its condition holds, and says which it entered", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(), LampRows, NothingDeclared);

    REQUIRE_FALSE(machine.advance(0.01f, Off, NothingDeclared).has_value());
    REQUIRE(machine.activeName() == "dark");

    REQUIRE(machine.advance(0.01f, On, NothingDeclared) == std::optional<std::size_t>(1));
    REQUIRE(machine.activeName() == "lit");

    REQUIRE_FALSE(machine.advance(0.01f, On, NothingDeclared).has_value());
}

TEST_CASE("Goes back once the condition no longer holds", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(), LampRows, NothingDeclared);
    machine.advance(0.01f, On, NothingDeclared);

    machine.advance(0.01f, Off, NothingDeclared);

    REQUIRE(machine.activeName() == "dark");
}

TEST_CASE("Takes one transition a tick, from the state it is in", "[StateMachine]")
{
    LampData data = aLamp();
    data.states.push_back(aState("burnt out"));
    data.transitions.push_back(aTransition("lit", "burnt out", "switchedOn", true));
    StateMachine<Lamp> machine(data, LampRows, NothingDeclared);

    machine.advance(0.01f, On, NothingDeclared);
    REQUIRE(machine.activeName() == "lit");

    machine.advance(0.01f, On, NothingDeclared);
    REQUIRE(machine.activeName() == "burnt out");
}

TEST_CASE("Waits for a condition to hold for as long as it asks", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(1.0f), LampRows, NothingDeclared);

    hold(machine, On, 50);
    REQUIRE(machine.activeName() == "dark");

    hold(machine, On, 60);
    REQUIRE(machine.activeName() == "lit");
}

TEST_CASE("A condition that stops holding starts its wait over", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(1.0f), LampRows, NothingDeclared);

    hold(machine, On, 90);
    hold(machine, Off, 1);
    hold(machine, On, 90);
    REQUIRE(machine.activeName() == "dark");

    hold(machine, On, 20);
    REQUIRE(machine.activeName() == "lit");
}

TEST_CASE("Coming back to a state starts its waits over", "[StateMachine]")
{
    FactsData declared;
    declared["tapped"] = false;
    LampData data = aLamp(1.0f);
    data.states.push_back(aState("tapped"));
    data.transitions.push_back(aTransition("dark", "tapped", "tapped", true));
    data.transitions.push_back(aTransition("tapped", "dark", "tapped", false));
    StateMachine<Lamp> machine(data, LampRows, declared);
    auto holdTapped = [&](bool tapped, int ticks)
    {
        declared["tapped"] = tapped;
        for (int tick = 0; tick < ticks; ++tick)
            machine.advance(0.01f, On, declared);
    };

    holdTapped(false, 60);
    holdTapped(true, 1);
    REQUIRE(machine.activeName() == "tapped");
    holdTapped(false, 1);
    REQUIRE(machine.activeName() == "dark");

    holdTapped(false, 50);
    REQUIRE(machine.activeName() == "dark");
}

TEST_CASE("A state on cooldown is not re-entered until it has passed", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(0.0f, 1.0f), LampRows, NothingDeclared);
    hold(machine, On, 1);
    hold(machine, Off, 1);
    REQUIRE(machine.activeName() == "dark");

    hold(machine, On, 90);
    REQUIRE(machine.activeName() == "dark");

    hold(machine, On, 20);
    REQUIRE(machine.activeName() == "lit");
}

TEST_CASE("A transition to a state it does not have is passed over", "[StateMachine]")
{
    LampData data = aLamp();
    data.transitions.insert(
        data.transitions.begin(), aTransition("dark", "ghost", "switchedOn", true));
    StateMachine<Lamp> machine(data, LampRows, NothingDeclared);

    machine.advance(0.01f, On, NothingDeclared);

    REQUIRE(machine.activeName() == "lit");
}

TEST_CASE("Resetting goes back to the first state", "[StateMachine]")
{
    StateMachine<Lamp> machine(aLamp(), LampRows, NothingDeclared);
    machine.advance(0.01f, On, NothingDeclared);

    machine.reset();

    REQUIRE(machine.activeName() == "dark");
}

TEST_CASE("Given no states at all, it is in none and never moves", "[StateMachine]")
{
    StateMachine<Lamp> machine(LampData{}, LampRows, NothingDeclared);

    machine.reset();

    REQUIRE(machine.activeName().empty());
    REQUIRE_FALSE(machine.advance(0.01f, On, NothingDeclared).has_value());
}

TEST_CASE(
    "A declared fact holds when it is equal: a number when equal, a name when it matches",
    "[StateMachine]")
{
    FactsData declared;
    declared["hits"] = 0.0f;
    declared["mood"] = std::string("calm");
    TransitionData provoked = aTransition("dark", "lit", "hits", 2.0f);
    provoked.when["mood"] = std::string("sour");
    StateMachine<Lamp> machine(
        LampData{{aState("dark"), aState("lit")}, {provoked}}, LampRows, declared);

    declared["hits"] = 2.0f;
    machine.advance(0.01f, Off, declared);
    REQUIRE(machine.activeName() == "dark");

    declared["mood"] = std::string("sour");
    machine.advance(0.01f, Off, declared);
    REQUIRE(machine.activeName() == "lit");
}

TEST_CASE("A transition asking about a fact nobody declares is refused", "[StateMachine]")
{
    LampData data = aLamp();
    data.transitions.front().when["snowing"] = true;

    REQUIRE_THROWS_WITH(
        StateMachine<Lamp>(data, LampRows, NothingDeclared),
        "The transition from \"dark\" to \"lit\" asks about \"snowing\", and there is no such "
        "fact");
}

TEST_CASE("A transition asking with the wrong kind of value is refused", "[StateMachine]")
{
    LampData data = aLamp();
    data.transitions.front().when["switchedOn"] = 3.0f;
    REQUIRE_THROWS_WITH(
        StateMachine<Lamp>(data, LampRows, NothingDeclared),
        Catch::Matchers::ContainsSubstring("wants a yes or no"));

    FactsData declared;
    declared["mood"] = std::string("calm");
    data = aLamp();
    data.transitions.front().when["mood"] = true;
    REQUIRE_THROWS_WITH(
        StateMachine<Lamp>(data, LampRows, declared),
        Catch::Matchers::ContainsSubstring("wants a name"));
}

TEST_CASE("A declared fact missing when it is asked is an error", "[StateMachine]")
{
    FactsData declared;
    declared["near"] = false;
    StateMachine<Lamp> machine(
        LampData{{aState("dark"), aState("lit")}, {aTransition("dark", "lit", "near", true)}},
        LampRows,
        declared);

    REQUIRE_THROWS_AS(machine.advance(0.01f, Off, NothingDeclared), std::runtime_error);
}
