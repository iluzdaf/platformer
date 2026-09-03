#pragma once

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "npc/npc_spawn_data.hpp"

class TileMap;

std::pair<glm::vec2, glm::vec2> walkBetween(const TileMap &tileMap, const PatrolData &patrol);
