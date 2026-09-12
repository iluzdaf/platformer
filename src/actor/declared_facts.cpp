#include <optional>
#include <stdexcept>
#include <string>
#include "actor/behaviors/behavior_facts.hpp"
#include "actor/declared_facts.hpp"
#include "actor/fading_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"

namespace
{
    constexpr float SaidLingersFor = 0.5f;
}

DeclaredFacts::DeclaredFacts(const FactsData &declared) : declared(declared), known(declared)
{
    for (const auto &[name, value] : declared)
        if (rowNamed(behaviorRows(), name))
            throw std::runtime_error(
                "\"" + name + "\" is a fact the engine answers, and cannot be declared");
}

const FactsData &DeclaredFacts::all() const
{
    return known;
}

const Asked &DeclaredFacts::fact(const std::string &name) const
{
    auto found = known.find(name);
    if (found == known.end())
        throw std::runtime_error("\"" + name + "\" is not a declared fact");

    return found->second;
}

void DeclaredFacts::say(const std::string &name, const Asked &value)
{
    if (std::optional<std::string> why = whyNotDeclared(declared, name, value))
        throw std::runtime_error(*why);

    known[name] = value;
}

void DeclaredFacts::fact(const std::string &name, const Asked &value)
{
    say(name, value);
}

void DeclaredFacts::event(const std::string &name, const Asked &value)
{
    say(name, value);
    saidForTheTick.push_back(name);
    lately.said(name, value, SaidLingersFor);
}

const FadingFacts &DeclaredFacts::saidLately() const
{
    return lately;
}

void DeclaredFacts::fade(float deltaTime)
{
    lately.update(deltaTime);
}

void DeclaredFacts::forgetTheTick()
{
    for (const std::string &name : saidForTheTick)
        known[name] = declared.at(name);

    saidForTheTick.clear();
}
