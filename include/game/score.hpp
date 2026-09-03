#pragma once
#include <signals.hpp>

class Score
{
public:
    void add(int delta);
    fteng::signal<void(int)> onChanged;
    int total() const;

private:
    int score = 0;
};
