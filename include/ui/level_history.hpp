#pragma once

#include <cstddef>
#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "game/level_data.hpp"

struct LevelAsItWas
{
    LevelData levelData;
    glm::vec2 movingThePlayerBack{0.0f};
};

inline constexpr std::size_t StepsRemembered = 64;

class LevelHistory
{
public:
    void remembers(const LevelData &before, const glm::vec2 &movingThePlayerBack = glm::vec2(0.0f));
    bool anythingToUndo() const;
    std::optional<LevelAsItWas> stepBack();
    void forgets();

private:
    std::vector<LevelAsItWas> steps;
};
