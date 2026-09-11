#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"
#include "ui/editor_section.hpp"

struct EditorStep
{
    EditorSection section = EditorSection::Level;
    std::optional<std::string> gameData;
    std::optional<LevelData> levelData;
    glm::vec2 movingThePlayerBack{0.0f};
};

inline constexpr std::size_t StepsRemembered = 64;

class EditorHistory
{
public:
    void remembers(EditorStep step);
    void remembers(const LevelData &before, const glm::vec2 &movingThePlayerBack = glm::vec2(0.0f));
    bool anythingToUndo() const;
    std::optional<EditorStep> stepBack();
    void forgets();

private:
    std::vector<EditorStep> steps;
};
