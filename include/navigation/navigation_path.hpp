#pragma once

#include <optional>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

class NavigationGraph;

std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId);

std::vector<int> roundTripFrom(const NavigationGraph &navigationGraph, int fromId);

std::vector<int> walkableFrom(const NavigationGraph &navigationGraph, int fromId);

std::optional<int> nearestNodeTo(const NavigationGraph &navigationGraph, glm::vec2 position);

std::optional<int> nodeUnderfoot(const NavigationGraph &navigationGraph, glm::vec2 position);

glm::vec2 placeOnTheRun(const NavigationGraph &navigationGraph, glm::vec2 asked);

bool onTheSameRun(const NavigationGraph &navigationGraph, glm::vec2 here, glm::vec2 there);

bool canPatrolBetween(const NavigationGraph &navigationGraph, glm::vec2 from, glm::vec2 to);
