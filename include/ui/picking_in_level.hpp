#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "ui/actors_in_level.hpp"

class Level;
struct AABB;

ActorShown whatIsAt(const Level &level, const AABB &playerBox, glm::vec2 at);
