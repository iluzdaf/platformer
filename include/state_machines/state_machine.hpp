#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"
#include "state_machines/state_machine_data.hpp"

template <class Context> class StateMachine
{
public:
    template <class Does>
    StateMachine(
        const StateMachineData<Does> &data,
        std::span<const FactRow<Context>> rows,
        const FactsData &declared)
        : rows(rows), transitions(data.transitions), heldFor(data.transitions.size(), 0.0f),
          sinceLeft(data.states.size(), NeverLeft)
    {
        for (const StateData<Does> &state : data.states)
            states.push_back({state.name, state.cooldown});

        for (const TransitionData &transition : transitions)
            for (const auto &[name, asked] : transition.when)
                if (std::optional<std::string> why =
                        whyNotAsked(name, asked, kindKnown(name, declared)))
                    throw std::runtime_error(
                        "The transition from \"" + transition.from + "\" to \"" + transition.to +
                        "\" " + *why);
    }

    std::optional<std::size_t> advance(
        float deltaTime,
        const Context &context,
        const FactsData &declared)
    {
        if (states.empty())
            return std::nullopt;

        for (float &since : sinceLeft)
            since += deltaTime;

        for (std::size_t index = 0; index < transitions.size(); ++index)
        {
            const TransitionData &transition = transitions[index];
            if (transition.from != states[activeState].name)
                continue;

            if (!holds(transition.when, context, declared))
            {
                heldFor[index] = 0.0f;
                continue;
            }

            heldFor[index] += deltaTime;
            if (heldFor[index] < transition.after)
                continue;

            std::optional<std::size_t> destination = stateNamed(transition.to);
            if (!destination || *destination == activeState)
                continue;

            if (sinceLeft[*destination] < states[*destination].cooldown)
                continue;

            enter(*destination);
            return destination;
        }

        return std::nullopt;
    }

    void reset()
    {
        if (!states.empty())
            enter(0);
    }

    std::size_t active() const
    {
        return activeState;
    }

    std::string_view activeName() const
    {
        return states.empty() ? std::string_view{} : states[activeState].name;
    }

private:
    static constexpr float NeverLeft = 1e9f;

    struct State
    {
        std::string name;
        float cooldown;
    };

    std::span<const FactRow<Context>> rows;
    std::vector<State> states;
    std::vector<TransitionData> transitions;
    std::size_t activeState = 0;
    std::vector<float> heldFor;
    std::vector<float> sinceLeft;

    std::optional<AskedKind> kindKnown(const std::string &name, const FactsData &declared) const
    {
        if (const FactRow<Context> *row = rowNamed(rows, name))
            return row->kind;

        auto fact = declared.find(name);
        if (fact != declared.end())
            return kindOf(fact->second);

        return std::nullopt;
    }

    bool holds(const TransitionWhenData &when, const Context &context, const FactsData &declared)
        const
    {
        for (const auto &[name, asked] : when)
        {
            if (const FactRow<Context> *row = rowNamed(rows, name))
            {
                if (!row->holds(asked, context))
                    return false;

                continue;
            }

            auto fact = declared.find(name);
            if (fact == declared.end())
                throw std::runtime_error(
                    "A condition asks about \"" + name + "\", and there is no such fact");

            if (fact->second != asked)
                return false;
        }

        return true;
    }

    std::optional<std::size_t> stateNamed(std::string_view name) const
    {
        for (std::size_t state = 0; state < states.size(); ++state)
            if (states[state].name == name)
                return state;

        return std::nullopt;
    }

    void enter(std::size_t state)
    {
        sinceLeft[activeState] = 0.0f;
        activeState = state;
        heldFor.assign(transitions.size(), 0.0f);
    }
};
