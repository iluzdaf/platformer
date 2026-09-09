#include <algorithm>
#include <stdexcept>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/actor_animation_data.hpp"
#include "animations/animator_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/hit.hpp"
#include "actor/hurting.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "physics/aabb.hpp"
#include "actor/observing.hpp"
#include "actor/observed.hpp"
#include "actor/decided.hpp"
#include "animations/frame_animation_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animator.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/fact_rows.hpp"
#include "actor/actor_behavior_context.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_profile.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "input/input_intentions.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/noise.hpp"
#include <optional>
#include <memory>
#include <utility>

namespace
{
    bool attackClipSaysWhenToStrike(const ActorAnimationData &animations)
    {
        const FrameAnimationData *attack = clipNamed(animations, AttackClip);
        if (!attack || attack->loops)
            return false;

        return std::ranges::any_of(
            attack->cues, [](const FrameCueData &cue) { return cue.name == StrikeCue; });
    }

    bool hasPicturesToChooseFrom(const ActorAnimationData &animations)
    {
        for (const auto &[name, clip] : animations.clips)
            if (name != IdleClip)
                return true;

        return false;
    }

    void refuseALadderToNowhere(const ActorAnimationData &animations)
    {
        for (const AnimationTransitionData &rung : animations.ladder.transitions)
        {
            if (!rung.from.empty() && !clipNamed(animations, rung.from))
                throw std::runtime_error(
                    "The ladder leaves from \"" + rung.from + "\", and there is no such clip");

            if (!clipNamed(animations, rung.to))
                throw std::runtime_error(
                    "The ladder goes to \"" + rung.to + "\", and there is no such clip");

            if (std::optional<std::string> why = whyNotAsked(rung.when, animatorRows()))
                throw std::runtime_error("The rung to \"" + rung.to + "\" " + *why);
        }
    }
}

Actor::Actor(const ActorData &data)
    : abilities(data.motionData), physicsBody(data.physicsBodyData),
      animator(data.animationData.ladder), navigationProfile(buildNavigationProfile(data)),
      hp(data.healthData)
{
    sheet = data.sheet;
    actorState.size = drawnSizeOf(data);
    if (actorState.size.x <= 0.0f || actorState.size.y <= 0.0f)
        throw std::runtime_error("An actor drawn as nothing is one nobody can see");

    animator.add(std::string(IdleClip), FrameAnimation(FrameAnimationData{}));
    for (const auto &[name, clip] : data.animationData.clips)
        animator.add(name, FrameAnimation(clip));

    refuseALadderToNowhere(data.animationData);

    if (hasPicturesToChooseFrom(data.animationData) &&
        data.animationData.ladder.transitions.empty())
        throw std::runtime_error(
            "An actor with pictures to choose from must have a ladder to choose them by");

    const std::optional<SwingAbilityData> &swing = data.motionData.swingAbilityData;
    if (swing && !attackClipSaysWhenToStrike(data.animationData))
        throw std::runtime_error(
            "A swing needs an attack clip that plays once and cues " + std::string(StrikeCue));
}

void Actor::postFixedUpdate()
{
}

void Actor::beginFrame()
{
    observations.contacts = contactsForANewFrame(observations.contacts);
}

void Actor::fixedUpdate(
    float deltaTime,
    const Level &level,
    std::optional<glm::vec2> threatFeet,
    std::span<const Noise> noises)
{
    const TileMap &tileMap = level.getTileMap();
    hp.update(deltaTime);
    observations.alive = hp.alive();
    ActorBehaviorContext context =
        behaviorContext(level.graphFor(navigationProfile), threatFeet, noises);
    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, context) : InputIntentions();

    abilities.decide(deltaTime, inputIntentions, observations, decisions);
    observations.hits.clear();

    physicsBody.setVelocity(decisions.targetVelocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
    observations.previousVelocity = observations.velocity;
    observations.velocity = physicsBody.velocity();

    animator.animate(deltaTime, decisions, observations, stateName());

    if (!decisions.knockback.active)
        actorState.facingLeft = observations.velocity.x > 0
                                    ? false
                                    : (observations.velocity.x < 0 ? true : actorState.facingLeft);
    observations.facingLeft = actorState.facingLeft;
    actorState.currentFrame = animator.playing().frame();
    actorState.currentAnimation = animator.state();

    observations.cues = animator.takeCues();
    observations.animationFinished = animator.finished();
    for (const std::string &cue : observations.cues)
        onCue(cue);
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

std::optional<AABB> Actor::swingBox() const
{
    const SwingAbilityState &swing = decisions.swing;
    if (!swing.striking())
        return std::nullopt;

    AABB collider = physicsBody.aabb();
    float x = swing.direction < 0.0f ? collider.left() - swing.reach.x : collider.right();
    return AABB{glm::vec2(x, collider.center().y - swing.reach.y * 0.5f), swing.reach};
}

std::optional<Hurting> Actor::hurting() const
{
    if (!alive())
        return std::nullopt;

    const SwingAbilityState &swing = decisions.swing;
    if (std::optional<AABB> reach = swingBox())
        return Hurting{*reach, swing.damage, swing.direction};

    const PounceAbilityState &pounce = decisions.pounce;
    if (pounce.active)
        return Hurting{physicsBody.aabb(), pounce.damage, pounce.direction};

    return std::nullopt;
}

bool Actor::strike(Actor &target)
{
    std::optional<Hurting> hurting = this->hurting();
    if (!hurting || &target == this)
        return false;

    SwingAbilityState &swing = decisions.swing;
    if (swing.striking() && std::ranges::find(swing.struck, &target) != swing.struck.end())
        return false;

    if (!hurting->box.intersects(target.body().touchBox()))
        return false;

    float away = target.feet().x < feet().x ? -1.0f : 1.0f;
    float direction = hurting->direction != 0.0f ? hurting->direction : away;
    if (!target.takeHit(Hit{hurting->damage, glm::vec2(direction, 0.0f), false}))
        return false;

    if (swing.striking())
        swing.struck.push_back(&target);
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
    std::optional<glm::vec2> threatFeet,
    std::span<const Noise> noises) const
{
    return ActorBehaviorContext{
        navigationGraph,
        feet(),
        physicsBody.colliderSize(),
        threatFeet,
        observations.contacts,
        noises};
}
