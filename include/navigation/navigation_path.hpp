#pragma once

#include <optional>
#include <unordered_map>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

class NavigationGraph;
struct NavigationEdge;

float costOf(const NavigationGraph &navigationGraph, const NavigationEdge &edge);

std::vector<int> findPath(const NavigationGraph &navigationGraph, int fromId, int toId);

std::unordered_map<int, float> costsFrom(const NavigationGraph &navigationGraph, int fromId);

std::vector<int> roundTripFrom(const NavigationGraph &navigationGraph, int fromId);

std::vector<int> walkableFrom(const NavigationGraph &navigationGraph, int fromId);

bool connectedInContact(const NavigationGraph &navigationGraph, int fromId, int toId);

std::optional<int> nearestNodeTo(const NavigationGraph &navigationGraph, glm::vec2 position);
