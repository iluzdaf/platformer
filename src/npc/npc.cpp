#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include "npc/npc.hpp"
#include <stdexcept>
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/actor.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/hurting.hpp"
#include "physics/physics_body.hpp"

namespace
{
    bool canAttackWith(const ActorMotionData &motion, const std::string &attack)
    {
        if (attack == SwingAttack)
            return motion.swingAbilityData.has_value();
        if (attack == PounceAttack)
            return motion.pounceAbilityData.has_value();
        return false;
    }
}

Npc::Npc(const NpcSpawnData &spawn, const NpcData &npcData)
    : Actor(npcData.actorData), spawn(spawn), npcData(npcData)
{
    declare(npcData.facts);
    if (npcData.stateMachineBehaviorData)
    {
        for (const BehaviorStateData &state : npcData.stateMachineBehaviorData->states)
            if (const auto *attack = std::get_if<AttackBehaviorData>(&state.does);
                attack && !canAttackWith(npcData.actorData.motionData, attack->with))
                throw std::runtime_error(
                    "\"" + spawn.type + "\" attacks with \"" + attack->with + "\" in state \"" +
                    state.name + "\", and has no such ability");

        std::optional<std::pair<glm::vec2, glm::vec2>> walk;
        if (this->spawn.patrol)
            walk = std::pair(this->spawn.patrol->from, this->spawn.patrol->to);

        setBehavior(
            std::make_unique<StateMachineBehavior>(
                npcData.stateMachineBehaviorData.value(), walk, npcData.facts));
    }

    standAt(this->spawn.feet);
}

const NpcSpawnData &Npc::getSpawn() const
{
    return spawn;
}

const std::string &Npc::type() const
{
    return spawn.type;
}

float Npc::tuning(const std::string &name) const
{
    auto found = npcData.tuning.find(name);
    if (found == npcData.tuning.end())
        throw std::runtime_error("\"" + spawn.type + "\" has no tuning called \"" + name + "\"");

    return found->second;
}

void Npc::died()
{
    setBehavior(nullptr);
}

std::optional<Hurting> Npc::hurting() const
{
    if (std::optional<Hurting> attacking = Actor::hurting())
        return attacking;

    if (!alive() || npcData.contactDamage <= 0)
        return std::nullopt;

    return Hurting{body().aabb(), npcData.contactDamage, 0.0f};
}
