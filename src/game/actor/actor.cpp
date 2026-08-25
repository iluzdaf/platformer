#include "game/actor/actor.hpp"
#include "input/input_intentions.hpp"

Actor::Actor(const AgentData &agentData, const ActorAnimationData &animationData)
    : agent(agentData)
{
    animationManager.addAnimation(ActorAnimationState::Idle, SpriteAnimation(animationData.idleSpriteAnimationData));
    animationManager.addAnimation(ActorAnimationState::Walk, SpriteAnimation(animationData.walkSpriteAnimationData));
    animationManager.addAnimation(ActorAnimationState::Dash, SpriteAnimation(animationData.dashSpriteAnimationData));
    animationManager.addAnimation(ActorAnimationState::Jump, SpriteAnimation(animationData.jumpSpriteAnimationData));
    animationManager.addAnimation(ActorAnimationState::Fall, SpriteAnimation(animationData.fallSpriteAnimationData));
    animationManager.addAnimation(ActorAnimationState::WallSlide, SpriteAnimation(animationData.wallSlideSpriteAnimationData));
}

void Actor::postFixedUpdate()
{
}

void Actor::preFixedUpdate()
{
    agent.resetCollisionAABB();
}

void Actor::fixedUpdate(float deltaTime, const TileMap &tileMap)
{
    InputIntentions inputIntentions = decideIntentions(deltaTime, tileMap);

    agent.fixedUpdate(deltaTime, tileMap, inputIntentions);

    animationManager.update(deltaTime, agent.getState());

    const AgentState &agentState = agent.getState();
    actorState.facingLeft = agentState.velocity.x > 0 ? false : (agentState.velocity.x < 0 ? true : actorState.facingLeft);
    actorState.currentAnimationUVStart = animationManager.getCurrentAnimation().getUVStart();
    actorState.currentAnimationUVEnd = animationManager.getCurrentAnimation().getUVEnd();
    actorState.currentAnimationState = animationManager.getCurrentState();
}

const ActorState &Actor::getState() const
{
    return actorState;
}

const Agent &Actor::getAgent() const
{
    return agent;
}

void Actor::setPosition(const glm::vec2 &position)
{
    agent.setPosition(position);
}

glm::vec2 Actor::getFootOffset() const
{
    const PhysicsBody &physicsBody = agent.getPhysicsBody();
    return physicsBody.getColliderOffset() +
           glm::vec2(physicsBody.getColliderSize().x * 0.5f, physicsBody.getColliderSize().y);
}
