#include <optional>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/actor_animation_data.hpp"
#include "actor/actor_animation_state.hpp"
#include "animations/frame_animation.hpp"
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
    sheet = data.sheet;
    actorState.size = data.size;

    const ActorAnimationData &animationData = data.animationData;

    animationManager.addAnimation(ActorAnimationState::Idle, FrameAnimation(animationData.idle));
    if (animationData.walk)
        animationManager.addAnimation(
            ActorAnimationState::Walk, FrameAnimation(animationData.walk.value()));
    if (animationData.dash)
        animationManager.addAnimation(
            ActorAnimationState::Dash, FrameAnimation(animationData.dash.value()));
    if (animationData.jump)
        animationManager.addAnimation(
            ActorAnimationState::Jump, FrameAnimation(animationData.jump.value()));
    if (animationData.fall)
        animationManager.addAnimation(
            ActorAnimationState::Fall, FrameAnimation(animationData.fall.value()));
    if (animationData.wallSlide)
        animationManager.addAnimation(
            ActorAnimationState::WallSlide, FrameAnimation(animationData.wallSlide.value()));
}

void Actor::postFixedUpdate()
{
}

void Actor::preFixedUpdate()
{
    motion.beginFrame();
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
    actorState.currentFrame = animationManager.getCurrentAnimation().getCurrentFrame();
    actorState.currentAnimationState = animationManager.getCurrentState();
}

const SheetData &Actor::getSheet() const
{
    return sheet;
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

std::optional<int> Actor::getCurrentNodeId() const
{
    return behavior ? behavior->getCurrentNodeId() : std::nullopt;
}

std::optional<int> Actor::getTargetNodeId() const
{
    return behavior ? behavior->getTargetNodeId() : std::nullopt;
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
        motion.getState().contacts};
}
