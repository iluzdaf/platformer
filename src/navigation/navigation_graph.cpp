#include "navigation/navigation_graph.hpp"

void NavigationGraph::addNode(int id, glm::vec2 position)
{
    addNode(NavigationNode{id, position});
}

void NavigationGraph::addNode(NavigationNode node)
{
    if (nodes.contains(node.id))
        throw std::runtime_error("Duplicate node ID");
    nodes[node.id] = node;
}

void NavigationGraph::addEdge(
    int fromId,
    int toId,
    EdgeType type)
{
    addEdge({fromId, toId, type});
}

void NavigationGraph::addEdge(NavigationEdge edge)
{
    if (!nodes.contains(edge.fromId) || !nodes.contains(edge.toId))
        throw std::runtime_error("Cannot add edge: missing node(s)");

    if (edge.fromId == edge.toId)
    {
        throw std::runtime_error("Cannot add self-loop edge: fromId == toId");
    }

    edges.push_back(edge);
    adjacency[edge.fromId].push_back(edge);
}

NavigationNode NavigationGraph::getNode(int id) const
{
    auto it = nodes.find(id);
    if (it == nodes.end())
        throw std::runtime_error("Invalid node ID");
    return it->second;
}

const std::unordered_map<int, NavigationNode> &NavigationGraph::getNodes() const
{
    return nodes;
}

const std::vector<NavigationEdge> &NavigationGraph::getEdges() const
{
    return edges;
}

const std::vector<NavigationEdge> &NavigationGraph::getOutgoingEdges(int id) const
{
    static const std::vector<NavigationEdge> empty;

    auto it = adjacency.find(id);
    if (it != adjacency.end())
        return it->second;

    return empty;
}

void NavigationGraph::clear()
{
    nodes.clear();
    edges.clear();
    adjacency.clear();
}

bool NavigationGraph::hasNodeAtPosition(glm::vec2 position, float epsilon) const
{
    for (const auto &[id, node] : nodes)
    {
        if (glm::distance(node.position, position) < epsilon)
        {
            return true;
        }
    }
    return false;
}