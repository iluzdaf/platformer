#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

class Level;
class Npc;

std::optional<std::size_t> drawNpcsInLevel(
    const Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs);
