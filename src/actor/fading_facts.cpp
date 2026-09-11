#include <string>
#include <unordered_map>
#include "actor/fading_facts.hpp"
#include "conditions/asked.hpp"

void FadingFacts::said(const std::string &name, const Asked &value, float seconds)
{
    lately[name] = Fading{value, seconds};
}

void FadingFacts::update(float deltaTime)
{
    for (auto it = lately.begin(); it != lately.end();)
    {
        it->second.secondsLeft -= deltaTime;
        if (it->second.secondsLeft <= 0.0f)
            it = lately.erase(it);
        else
            ++it;
    }
}

const std::unordered_map<std::string, FadingFacts::Fading> &FadingFacts::all() const
{
    return lately;
}
