#include <optional>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/editor_history.hpp"
#include "ui/editor_section.hpp"
#include "game/level_data.hpp"

void EditorHistory::remembers(EditorStep step)
{
    steps.push_back(std::move(step));
    if (steps.size() > StepsRemembered)
        steps.erase(steps.begin());
}

void EditorHistory::remembers(const LevelData &before, const glm::vec2 &movingThePlayerBack)
{
    remembers(EditorStep{EditorSection::Level, std::nullopt, before, movingThePlayerBack});
}

bool EditorHistory::anythingToUndo() const
{
    return !steps.empty();
}

std::optional<EditorStep> EditorHistory::stepBack()
{
    if (steps.empty())
        return std::nullopt;

    EditorStep back = std::move(steps.back());
    steps.pop_back();

    return back;
}

void EditorHistory::forgets()
{
    steps.clear();
}
