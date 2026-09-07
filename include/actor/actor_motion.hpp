#pragma once

#include "actor/actor_motion_data.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/ability_system.hpp"

class TileMap;
class PhysicsBody;
struct InputIntentions;
struct Hit;

class ActorMotion
{
public:
    explicit ActorMotion(const ActorMotionData &data);
    void applyMovement(float deltaTime, const InputIntentions &inputIntentions);
    void readContacts(const PhysicsBody &physicsBody, const TileMap &tileMap);
    void readMotion(const PhysicsBody &physicsBody);
    void beginFrame();
    void pushedBy(const Hit &hit);
    const ActorMotionState &getState() const;
    const Observed &observed() const;

private:
    ActorMotionState state;
    Observed observations;
    AbilitySystem abilitySystem;
};
