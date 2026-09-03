#pragma once

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "npc/npc_spawn_data.hpp"

class TileMap;

PatrolData beatBetween(const TileMap &tileMap, glm::ivec2 fromTile, glm::ivec2 toTile);

std::pair<glm::ivec2, glm::ivec2> tilesOfBeat(const TileMap &tileMap, const PatrolData &beat);
