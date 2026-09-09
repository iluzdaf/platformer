#include <memory>
#include <optional>
#include <string>
#include <utility>
#include "npc/npc.hpp"
#include <stdexcept>
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/actor_motion_data.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/decided.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/actor.hpp"
#include "actor/behaviors/state_machine_behavior.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

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
    if (npcData.stateMachineBehaviorData)
    {
        for (const BehaviorStateData &state : npcData.stateMachineBehaviorData->states)
            if (state.attackBehaviorData &&
                !canAttackWith(npcData.actorData.motionData, state.attackBehaviorData->with))
                throw std::runtime_error(
                    "\"" + spawn.type + "\" attacks with \"" + state.attackBehaviorData->with +
                    "\" in state \"" + state.name + "\", and has no such ability");

        std::optional<std::pair<glm::vec2, glm::vec2>> walk;
        if (this->spawn.patrol)
            walk = std::pair(this->spawn.patrol->from, this->spawn.patrol->to);

        setBehavior(
            std::make_unique<StateMachineBehavior>(npcData.stateMachineBehaviorData.value(), walk));
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

void Npc::died()
{
    setBehavior(nullptr);
}

int Npc::contactDamage() const
{
    const PounceAbilityState &pounce = decided().pounce;
    return pounce.active ? pounce.damage : npcData.contactDamage;
}
