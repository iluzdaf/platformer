#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <unordered_map>
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_node.hpp"

class NavigationGraph
{
public:
    void addNode(int id, glm::vec2 position);
    void addNode(NavigationNode node);
    void addEdge(int fromId, int toId, EdgeType type);
    void addEdge(NavigationEdge edge);
    NavigationNode getNode(int id) const;
    const std::unordered_map<int, NavigationNode> &getNodes() const;
    const std::vector<NavigationEdge> &getEdges() const;
    const std::vector<NavigationEdge> &getOutgoingEdges(int id) const;
    void clear();
    bool hasNodeAtPosition(glm::vec2 position, float epsilon = 0.1f) const;

private:
    std::unordered_map<int, NavigationNode> nodes;
    std::vector<NavigationEdge> edges;
    std::unordered_map<int, std::vector<NavigationEdge>> adjacency;
};