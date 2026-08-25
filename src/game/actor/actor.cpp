#include "game/actor/actor.hpp"
#include "input/input_intentions.hpp"
#include "game/tile_map/tile_map.hpp"

Actor::Actor(const AgentData &agentData, const ActorAnimationData &animationData)
    : agent(agentData),
      physicsBody(agentData.physicsBodyData)
{
    actorState.size = agentData.size;

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

    agent.applyMovement(deltaTime, inputIntentions);

    physicsBody.setVelocity(agent.getState().targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    agent.readContacts(physicsBody, tileMap);
    agent.readMotion(physicsBody);

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

const PhysicsBody &Actor::getPhysicsBody() const
{
    return physicsBody;
}

const glm::vec2 &Actor::getPosition() const
{
    return physicsBody.getPosition();
}

void Actor::setPosition(const glm::vec2 &position)
{
    physicsBody.setPosition(position);
}

glm::vec2 Actor::getFootOffset() const
{
    return physicsBody.getColliderOffset() +
           glm::vec2(physicsBody.getColliderSize().x * 0.5f, physicsBody.getColliderSize().y);
}
