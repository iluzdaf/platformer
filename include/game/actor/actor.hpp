#pragma once

#include <memory>
#include "game/actor/actor_state.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "game/actor/actor_motion.hpp"
#include "game/actor/actor_behavior.hpp"
#include "animations/animation_manager.hpp"
#include "physics/physics_body.hpp"

class TileMap;
class NavigationGraph;

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
    void setBehavior(std::unique_ptr<ActorBehavior> newBehavior);
    ActorBehaviorContext behaviorContext(const NavigationGraph &navigationGraph) const;
    ActorBehaviorContext behaviorContextAt(
        const glm::vec2 &position,
        const NavigationGraph &navigationGraph) const;

    ActorMotion motion;
    PhysicsBody physicsBody;
    AnimationManager animationManager;
    ActorState actorState;
    std::unique_ptr<ActorBehavior> behavior;
};
