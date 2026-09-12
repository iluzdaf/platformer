#pragma once

#include <string>
#include <vector>
#include "actor/fading_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"

class DeclaredFacts
{
public:
    explicit DeclaredFacts(const FactsData &declared = {});

    const FactsData &all() const;
    const Asked &fact(const std::string &name) const;
    void fact(const std::string &name, const Asked &value);
    void event(const std::string &name, const Asked &value);
    const FadingFacts &saidLately() const;

    void fade(float deltaTime);
    void forgetTheTick();

private:
    void say(const std::string &name, const Asked &value);
    FactsData declared;
    FactsData known;
    std::vector<std::string> saidForTheTick;
    FadingFacts lately;
};
