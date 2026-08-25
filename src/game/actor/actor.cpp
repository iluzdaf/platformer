#include "game/actor/actor.hpp"
#include "input/input_intentions.hpp"
#include "game/tile_map/tile_map.hpp"

Actor::Actor(const ActorMotionData &motionData, const ActorAnimationData &animationData)
    : motion(motionData),
      physicsBody(motionData.physicsBodyData)
{
    actorState.size = motionData.size;

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
    motion.resetCollisionAABB();
}

void Actor::fixedUpdate(float deltaTime, const TileMap &tileMap)
{
    InputIntentions inputIntentions = decideIntentions(deltaTime, tileMap);

    motion.applyMovement(deltaTime, inputIntentions);

    physicsBody.setVelocity(motion.getState().targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    motion.readContacts(physicsBody, tileMap);
    motion.readMotion(physicsBody);

    animationManager.update(deltaTime, motion.getState());

    const ActorMotionState &motionState = motion.getState();
    actorState.facingLeft = motionState.velocity.x > 0 ? false : (motionState.velocity.x < 0 ? true : actorState.facingLeft);
    actorState.currentAnimationUVStart = animationManager.getCurrentAnimation().getUVStart();
    actorState.currentAnimationUVEnd = animationManager.getCurrentAnimation().getUVEnd();
    actorState.currentAnimationState = animationManager.getCurrentState();
}

const ActorState &Actor::getState() const
{
    return actorState;
}

const ActorMotion &Actor::getMotion() const
{
    return motion;
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
