#include "game/scoring_system.hpp"

void ScoringSystem::addScore(int delta)
{
    score += delta;
    onScoreChanged(score);
}

int ScoringSystem::getScore() const
{
    return score;
}