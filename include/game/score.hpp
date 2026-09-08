#pragma once
#include "events/event.hpp"

class Score
{
public:
    void add(int delta);
    Event<Score, int> onChanged;
    int total() const;

private:
    int score = 0;
};
