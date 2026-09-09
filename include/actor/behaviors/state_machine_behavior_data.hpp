#pragma once

#include <array>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <glaze/glaze.hpp>
#include "actor/behaviors/attack_behavior_data.hpp"
#include "conditions/asked.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"

using BehaviorDoes = std::variant<
    IdleBehaviorData,
    PatrolBehaviorData,
    FleeBehaviorData,
    ChaseBehaviorData,
    AttackBehaviorData>;

template <> struct glz::meta<BehaviorDoes>
{
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr std::string_view tag = "kind";
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto ids = std::array{"idle", "patrol", "flee", "chase", "attack"};
};

struct BehaviorStateData
{
    std::string name;
    BehaviorDoes does;
    float cooldown = 0.0f;

    bool operator==(const BehaviorStateData &) const = default;
};

struct BehaviorTransitionData
{
    std::string from;
    std::string to;
    BehaviorWhen when;
    float after = 0.0f;

    bool operator==(const BehaviorTransitionData &) const = default;
};

struct StateMachineBehaviorData
{
    std::vector<BehaviorStateData> states;
    std::vector<BehaviorTransitionData> transitions;

    bool operator==(const StateMachineBehaviorData &) const = default;
};
