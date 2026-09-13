#pragma once

#include "actor/behaviors/state_machine_behavior_data.hpp"
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/state_machine_shown.hpp"

struct AnimationRuleData;

struct GraphNode
{
    std::string name;
    std::string words;
    bool hub = false;

    bool operator==(const GraphNode &) const = default;
};

struct GraphEdge
{
    std::string from;
    std::string to;
    std::string words;

    bool operator==(const GraphEdge &) const = default;
};

struct GraphShown
{
    std::vector<GraphNode> nodes;
    std::vector<GraphEdge> edges;

    bool operator==(const GraphShown &) const = default;
};

std::optional<std::size_t> indexOfNode(const GraphShown &graph, std::string_view name);

bool goesBothWays(const GraphShown &graph, const GraphEdge &edge);

MachineShown stillAmong(MachineShown shown, const GraphShown &graph);

std::size_t nodesAroundIn(const GraphShown &graph);

float graphHeightFor(std::size_t ringCount);

std::vector<glm::vec2> placedAround(const GraphShown &graph, glm::vec2 centre, glm::vec2 radii);

std::string whenOf(const AnimationRuleData &rule);

GraphShown graphOf(const StateMachineBehaviorData &machine);
