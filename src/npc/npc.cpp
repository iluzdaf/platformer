#include <memory>
#include <optional>
#include <tuple>
#include <string>
#include <utility>
#include <variant>
#include "npc/npc.hpp"
#include <stdexcept>
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/scripted_behavior_data.hpp"
#include "actor/behaviors/state_script.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/charge_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/actor.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

namespace
{
    bool canAttackWith(const AbilitiesData &abilities, const std::string &attack)
    {
        if (attack == SwingAttack)
            return abilities.swing.has_value();
        if (attack == PounceAttack)
            return abilities.pounce.has_value();
        if (attack == ChargeAttack)
            return abilities.charge.has_value();
        return false;
    }
}

Npc::Npc(const NpcSpawnData &spawn, const NpcData &npcData)
    : Actor(npcData.actorData), spawn(spawn), npcData(npcData)
{
    declare(npcData.facts);
    setSenses(npcData.senses);
    if (npcData.stateMachineBehaviorData)
    {
        for (const BehaviorStateData &state : npcData.stateMachineBehaviorData->states)
        {
            if (const auto *attack = std::get_if<AttackBehaviorData>(&state.does);
                attack && !canAttackWith(npcData.actorData.abilities, attack->with))
                throw std::runtime_error(
                    "\"" + spawn.type + "\" attacks with \"" + attack->with + "\" in state \"" +
                    state.name + "\", and has no such ability");

            if (std::holds_alternative<ScriptedBehaviorData>(state.does) &&
                npcData.script.path.empty())
                throw std::runtime_error(
                    "\"" + spawn.type + "\" runs its state \"" + state.name +
                    "\" by script, and has no script");
        }

        std::optional<std::pair<glm::vec2, glm::vec2>> walk;
        if (this->spawn.patrol)
            walk = std::pair(this->spawn.patrol->from, this->spawn.patrol->to);

        setBehavior(
            std::make_unique<StateMachineBehavior>(
                npcData.stateMachineBehaviorData.value(), walk, npcData.facts, npcData.senses));
    }

    standAt(this->spawn.feet);
    std::ignore = onDeath.connect([this] { setBehavior(nullptr); });
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}

const NpcData &Npc::builtFrom() const
{
    return npcData;
}

const std::string &Npc::type() const
{
    return spawn.type;
}

void Npc::scriptStatesWith(std::unique_ptr<StateScript> script)
{
    stateScript = std::move(script);
    scriptBehaviorWith(stateScript.get());
}

float Npc::tuning(const std::string &name) const
{
    auto found = npcData.tuning.find(name);
    if (found == npcData.tuning.end())
        throw std::runtime_error("\"" + spawn.type + "\" has no tuning called \"" + name + "\"");

    return found->second;
}
