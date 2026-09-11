#include <optional>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/level_history.hpp"
#include "game/level_data.hpp"

void LevelHistory::remembers(const LevelData &before, const glm::vec2 &movingThePlayerBack)
{
    steps.push_back(LevelAsItWas{before, movingThePlayerBack});
    if (steps.size() > StepsRemembered)
        steps.erase(steps.begin());
}

bool LevelHistory::anythingToUndo() const
{
    return !steps.empty();
}

std::optional<LevelAsItWas> LevelHistory::stepBack()
{
    if (steps.empty())
        return std::nullopt;

    LevelAsItWas back = std::move(steps.back());
    steps.pop_back();

    return back;
}

void LevelHistory::forgets()
{
    steps.clear();
}
