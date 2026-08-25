#pragma once

#include "game/actor/actor_state.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "game/actor/actor_motion.hpp"
#include "animations/animation_manager.hpp"
#include "physics/physics_body.hpp"

class TileMap;
struct InputIntentions;

class Actor
{
public:
    virtual ~Actor() = default;
    void preFixedUpdate();
    void fixedUpdate(float deltaTime, const TileMap &tileMap);
    virtual void postFixedUpdate();
    const ActorState &getState() const;
    const ActorMotion &getMotion() const;
    const PhysicsBody &getPhysicsBody() const;
    const glm::vec2 &getPosition() const;
    void setPosition(const glm::vec2 &position);

protected:
    Actor(const ActorMotionData &motionData, const ActorAnimationData &animationData);
    virtual InputIntentions decideIntentions(float deltaTime, const TileMap &tileMap) = 0;

    ActorMotion motion;
    PhysicsBody physicsBody;
    AnimationManager animationManager;
    ActorState actorState;
};
