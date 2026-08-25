#pragma once

#include "game/actor/actor_state.hpp"
#include "game/actor/actor_animation_data.hpp"
#include "agent/agent.hpp"
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
    const Agent &getAgent() const;
    const PhysicsBody &getPhysicsBody() const;
    const glm::vec2 &getPosition() const;
    void setPosition(const glm::vec2 &position);
    glm::vec2 getFootOffset() const;

protected:
    Actor(const AgentData &agentData, const ActorAnimationData &animationData);
    virtual InputIntentions decideIntentions(float deltaTime, const TileMap &tileMap) = 0;

    Agent agent;
    PhysicsBody physicsBody;
    AnimationManager animationManager;
    ActorState actorState;
};
