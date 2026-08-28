#include <string_view>
#include "actor/actor.hpp"
#include "actor/actor_animation_data.hpp"
#include "actor/actor_animation_state.hpp"
#include "animations/sprite_animation.hpp"
#include "actor/actor_behavior_context.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "input/input_intentions.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include <optional>
#include <memory>
#include <utility>

Actor::Actor(const ActorData &data)
    : motion(data.motionData), physicsBody(data.physicsBodyData),
      navigationProfile(buildNavigationProfile(data))
{
    actorState.size = data.size;

    const ActorAnimationData &animationData = data.animationData;

    animationManager.addAnimation(
        ActorAnimationState::Idle, SpriteAnimation(animationData.idleSpriteAnimationData));
    if (animationData.walkSpriteAnimationData)
        animationManager.addAnimation(
            ActorAnimationState::Walk,
            SpriteAnimation(animationData.walkSpriteAnimationData.value()));
    if (animationData.dashSpriteAnimationData)
        animationManager.addAnimation(
            ActorAnimationState::Dash,
            SpriteAnimation(animationData.dashSpriteAnimationData.value()));
    if (animationData.jumpSpriteAnimationData)
        animationManager.addAnimation(
            ActorAnimationState::Jump,
            SpriteAnimation(animationData.jumpSpriteAnimationData.value()));
    if (animationData.fallSpriteAnimationData)
        animationManager.addAnimation(
            ActorAnimationState::Fall,
            SpriteAnimation(animationData.fallSpriteAnimationData.value()));
    if (animationData.wallSlideSpriteAnimationData)
        animationManager.addAnimation(
            ActorAnimationState::WallSlide,
            SpriteAnimation(animationData.wallSlideSpriteAnimationData.value()));
}

void Actor::postFixedUpdate()
{
}

void Actor::preFixedUpdate()
{
    motion.resetCollisionAABB();
}

void Actor::fixedUpdate(
    float deltaTime,
    const Level &level,
    std::optional<glm::vec2> threatPosition)
{
    const TileMap &tileMap = level.getTileMap();
    ActorBehaviorContext context =
        behaviorContext(level.graphFor(navigationProfile), threatPosition);
    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, context) : InputIntentions();

    motion.applyMovement(deltaTime, inputIntentions);

    physicsBody.setVelocity(motion.getState().targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    motion.readContacts(physicsBody, tileMap);
    motion.readMotion(physicsBody);

    animationManager.update(deltaTime, motion.getState());

    const ActorMotionState &motionState = motion.getState();
    actorState.facingLeft = motionState.velocity.x > 0
                                ? false
                                : (motionState.velocity.x < 0 ? true : actorState.facingLeft);
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

const NavigationProfile &Actor::getNavigationProfile() const
{
    return navigationProfile;
}

const glm::vec2 &Actor::getPosition() const
{
    return physicsBody.getPosition();
}

std::string_view Actor::getStateName() const
{
    return behavior ? behavior->getStateName() : std::string_view{};
}

void Actor::setPosition(const glm::vec2 &position)
{
    physicsBody.setPosition(position);

    if (behavior)
        behavior->reset();
}

void Actor::setBehavior(std::unique_ptr<ActorBehavior> newBehavior)
{
    behavior = std::move(newBehavior);
}

ActorBehaviorContext Actor::behaviorContext(
    const NavigationGraph &navigationGraph,
    std::optional<glm::vec2> threatPosition) const
{
    return ActorBehaviorContext{
        navigationGraph,
        physicsBody.getAABB().bottomCenter(),
        physicsBody.getColliderSize(),
        threatPosition,
        motion.getState().contacts.onGround};
}
