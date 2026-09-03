#include "game/score.hpp"

void Score::add(int delta)
{
    score += delta;
    onChanged(score);
}

int Score::total() const
{
    return score;
}
