#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include <glm/geometric.hpp>
#include "actor/abilities/mantle_ability_data.hpp"
#include "navigation/navigation_graph_steps.hpp"
#include "navigation/navigation_edge.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_profile.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    std::optional<float> secondsToWalk(const NavigationProfile &profile, float across)
    {
        if (!profile.abilities.move || profile.abilities.move->moveSpeed <= 0.0f)
            return std::nullopt;

        return std::abs(across) / profile.abilities.move->moveSpeed;
    }

    std::optional<float> secondsToClimb(
        const NavigationProfile &profile,
        NavigationNode from,
        NavigationNode to)
    {
        if (!profile.abilities.wallClimb || profile.abilities.wallClimb->climbSpeed <= 0.0f)
            return std::nullopt;

        float climbSpeed = profile.abilities.wallClimb->climbSpeed;
        float height = std::abs(to.feet.y - from.feet.y);
        bool ontoTheLedge = to.kind != NodeKind::OnWall && to.feet.y < from.feet.y;
        if (!ontoTheLedge || !profile.abilities.mantle)
            return height / climbSpeed;

        const MantleAbilityData &mantle = *profile.abilities.mantle;
        float pulledUp = mantle.mantleSpeed * mantle.mantleDuration * 0.5f;

        return std::max(height - pulledUp, 0.0f) / climbSpeed + mantle.mantleDuration;
    }

    std::optional<float> secondsInTheAir(
        const NavigationProfile &profile,
        const std::vector<glm::vec2> &path,
        NavigationNode to)
    {
        if (path.empty())
            return std::nullopt;

        float airborne = static_cast<float>(path.size() - 1) * PhysicsStep;
        float onwards = secondsToWalk(profile, to.feet.x - path.back().x).value_or(0.0f);

        return airborne + onwards;
    }
}

namespace navigation
{
    NavigationEdge timed(
        NavigationEdge edge,
        const NavigationGraph &navigationGraph,
        const NavigationProfile &profile)
    {
        NavigationNode from = navigationGraph.getNode(edge.fromId);
        NavigationNode to = navigationGraph.getNode(edge.toId);

        switch (edge.type)
        {
        case EdgeType::Walk:
            edge.duration = secondsToWalk(profile, to.feet.x - from.feet.x);
            break;
        case EdgeType::Climb:
            edge.duration = secondsToClimb(profile, from, to);
            break;
        case EdgeType::Jump:
        case EdgeType::Fall:
            edge.duration = secondsInTheAir(profile, edge.path, to);
            break;
        }

        return edge;
    }
}
