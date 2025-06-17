#pragma once
#include <signals.hpp>

class ScoringSystem
{
public:
    void addScore(int delta);
    fteng::signal<void(int)> onScoreChanged;
    int getScore() const;

private:
    int score = 0;
};