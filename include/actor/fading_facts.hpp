#pragma once

#include <string>
#include <unordered_map>
#include "conditions/asked.hpp"

class FadingFacts
{
public:
    struct Fading
    {
        Asked value;
        float secondsLeft = 0.0f;
    };

    void said(const std::string &name, const Asked &value, float seconds);
    void update(float deltaTime);
    const std::unordered_map<std::string, Fading> &all() const;

private:
    std::unordered_map<std::string, Fading> lately;
};
