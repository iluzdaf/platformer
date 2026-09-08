#include <optional>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/hit.hpp"
#include "actor/observing.hpp"
#include "actor/observed.hpp"
#include "actor/decided.hpp"
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
    : abilities(data.motionData), physicsBody(data.physicsBodyData),
      navigationProfile(buildNavigationProfile(data)), hp(data.healthData)
{
    sheet = data.sheet;
    actorState.size = data.size;

    const ActorAnimationData &animationData = data.animationData;

    animator.add(ActorAnimationState::Idle, FrameAnimation(animationData.idle));
    if (animationData.walk)
        animator.add(ActorAnimationState::Walk, FrameAnimation(animationData.walk.value()));
    if (animationData.dash)
        animator.add(ActorAnimationState::Dash, FrameAnimation(animationData.dash.value()));
    if (animationData.jump)
        animator.add(ActorAnimationState::Jump, FrameAnimation(animationData.jump.value()));
    if (animationData.fall)
        animator.add(ActorAnimationState::Fall, FrameAnimation(animationData.fall.value()));
    if (animationData.wallSlide)
        animator.add(
            ActorAnimationState::WallSlide, FrameAnimation(animationData.wallSlide.value()));
}

void Actor::postFixedUpdate()
{
}

void Actor::beginFrame()
{
    observations.contacts = contactsForANewFrame(observations.contacts);
}

void Actor::fixedUpdate(float deltaTime, const Level &level, std::optional<glm::vec2> threatFeet)
{
    const TileMap &tileMap = level.getTileMap();
    hp.update(deltaTime);
    ActorBehaviorContext context = behaviorContext(level.graphFor(navigationProfile), threatFeet);
    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, context) : InputIntentions();

    abilities.decide(deltaTime, inputIntentions, observations, decisions);
    observations.hits.clear();

    physicsBody.setVelocity(decisions.targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
    observations.previousVelocity = observations.velocity;
    observations.velocity = physicsBody.velocity();

    animator.animate(deltaTime, decisions, observations);

    if (!decisions.knockback.active)
        actorState.facingLeft = observations.velocity.x > 0
                                    ? false
                                    : (observations.velocity.x < 0 ? true : actorState.facingLeft);
    actorState.currentFrame = animator.playing().frame();
    actorState.currentAnimationState = animator.state();
}

const SheetData &Actor::drawnFrom() const
{
    return sheet;
}

const ActorState &Actor::state() const
{
    return actorState;
}

const Decided &Actor::decided() const
{
    return decisions;
}

const Observed &Actor::observed() const
{
    return observations;
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
        observations.hits.push_back(hit);
        hurt();
        onHurt();
    }
    else
    {
        died();
        onDeath();
    }

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
        navigationGraph, feet(), physicsBody.colliderSize(), threatFeet, observations.contacts};
}
