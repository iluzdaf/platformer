#pragma once

#include <array>
#include <string_view>
#include <variant>
#include <glaze/glaze.hpp>
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/chase_behavior_data.hpp"
#include "actor/behaviors/flee_behavior_data.hpp"
#include "actor/behaviors/idle_behavior_data.hpp"
#include "actor/behaviors/patrol_behavior_data.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "state_machines/state_machine_data.hpp"

using BehaviorDoes = std::variant<
    IdleBehaviorData,
    PatrolBehaviorData,
    FleeBehaviorData,
    ChaseBehaviorData,
    AttackBehaviorData,
    ScriptedBehaviorData>;

template <> struct glz::meta<BehaviorDoes>
{
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr std::string_view tag = "kind";
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto ids = std::array{"idle", "patrol", "flee", "chase", "attack", "script"};
};

using BehaviorStateData = StateData<BehaviorDoes>;
using StateMachineBehaviorData = StateMachineData<BehaviorDoes>;
