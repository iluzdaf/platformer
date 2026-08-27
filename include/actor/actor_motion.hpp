#pragma once

#include "actor/actor_motion_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/abilities/ability_system.hpp"

class TileMap;
class PhysicsBody;
struct InputIntentions;

class ActorMotion
{
public:
    explicit ActorMotion(const ActorMotionData &data);
    void applyMovement(float deltaTime, const InputIntentions &inputIntentions);
    void readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap);
    void readMotion(const PhysicsBody &physicsBody);
    void resetCollisionAABB();
    const ActorMotionState &getState() const;
    const AbilitySystem &getAbilitySystem() const;

private:
    ActorMotionState state;
    AbilitySystem abilitySystem;
};
