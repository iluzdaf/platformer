#pragma once

#include <optional>
#include <string>
#include <utility>

class LastAnswer
{
public:
    template <class Compute> std::optional<std::string> to(std::string question, Compute &&compute)
    {
        if (!asked || question != lastQuestion)
        {
            held = compute();
            lastQuestion = std::move(question);
            asked = true;
        }

        return held;
    }

private:
    bool asked = false;
    std::string lastQuestion;
    std::optional<std::string> held;
};
