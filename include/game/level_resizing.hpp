#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct LevelData;

enum class Side
{
    Left,
    Right,
    Above,
    Below
};

struct Resize
{
    Side side;
    bool larger;

    bool operator==(const Resize &) const = default;
};

glm::vec2 shiftOf(Resize resize, int tileSize);
LevelData resizedBy(Resize resize, const LevelData &level, int tileSize);
