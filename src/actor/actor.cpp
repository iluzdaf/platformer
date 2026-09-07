#include <optional>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/hit.hpp"
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
      navigationProfile(buildNavigationProfile(data)), hp(data.healthData)
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

void Actor::beginFrame()
{
    motion.beginFrame();
}

void Actor::fixedUpdate(float deltaTime, const Level &level, std::optional<glm::vec2> threatFeet)
{
    const TileMap &tileMap = level.getTileMap();
    hp.update(deltaTime);
    ActorBehaviorContext context = behaviorContext(level.graphFor(navigationProfile), threatFeet);
    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, context) : InputIntentions();

    motion.applyMovement(deltaTime, inputIntentions);

    physicsBody.setVelocity(motion.getState().targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    motion.readContacts(physicsBody, tileMap);
    motion.readMotion(physicsBody);

    animationManager.update(deltaTime, motion.getState(), motion.observed().contacts);

    const ActorMotionState &motionState = motion.getState();
    if (!motionState.knockback.active)
        actorState.facingLeft = motionState.velocity.x > 0
                                    ? false
                                    : (motionState.velocity.x < 0 ? true : actorState.facingLeft);
    actorState.currentFrame = animationManager.getCurrentAnimation().getCurrentFrame();
    actorState.currentAnimationState = animationManager.getCurrentState();
}

const SheetData &Actor::drawnFrom() const
{
    return sheet;
}

const ActorState &Actor::state() const
{
    return actorState;
}

const ActorMotion &Actor::moving() const
{
    return motion;
}

const PhysicsBody &Actor::body() const
{
    return physicsBody;
}

const NavigationProfile &Actor::profile() const
{
    return navigationProfile;
}

std::string_view Actor::stateName() const
{
    return behavior ? behavior->getStateName() : std::string_view{};
}

std::optional<int> Actor::currentNodeId() const
{
    return behavior ? behavior->getCurrentNodeId() : std::nullopt;
}

std::optional<int> Actor::targetNodeId() const
{
    return behavior ? behavior->getTargetNodeId() : std::nullopt;
}

glm::vec2 Actor::feet() const
{
    return physicsBody.aabb().bottomCenter();
}

void Actor::standAt(const glm::vec2 &newFeet)
{
    physicsBody.setPosition(newFeet - physicsBody.bottomCenterOffset());

    if (behavior)
        behavior->reset();
}

const Health &Actor::health() const
{
    return hp;
}

bool Actor::alive() const
{
    return hp.alive();
}

bool Actor::takeHit(const Hit &hit)
{
    if (!hp.takeHit(hit))
        return false;

    if (hp.alive())
    {
        motion.pushedBy(hit);
        hurt();
    }
    else
        died();

    return true;
}

void Actor::hurt()
{
}

void Actor::died()
{
}

void Actor::setBehavior(std::unique_ptr<ActorBehavior> newBehavior)
{
    behavior = std::move(newBehavior);
}

ActorBehaviorContext Actor::behaviorContext(
    const NavigationGraph &navigationGraph,
    std::optional<glm::vec2> threatFeet) const
{
    return ActorBehaviorContext{
        navigationGraph,
        feet(),
        physicsBody.colliderSize(),
        threatFeet,
        motion.observed().contacts};
}
