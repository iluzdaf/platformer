#pragma once

#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "events/event.hpp"
#include "actor/actor_state.hpp"
#include "assets/sheet_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/actor_behavior.hpp"
#include "animations/animator.hpp"
#include "physics/physics_body.hpp"
#include <limits>
#include "navigation/navigation_profile.hpp"
#include "actor/actor_behavior_context.hpp"
#include "conditions/asked.hpp"
#include "actor/fading_facts.hpp"
#include "conditions/facts.hpp"
#include "game/noise.hpp"
#include "actor/health.hpp"
#include "actor/hurting.hpp"
#include "physics/aabb.hpp"

class TileMap;
class Level;
class NavigationGraph;
struct Hit;

class Actor
{
public:
    virtual ~Actor() = default;
    void beginFrame();
    void fixedUpdate(
        float deltaTime,
        const Level &level,
        std::optional<glm::vec2> threatFeet = std::nullopt,
        std::span<const Noise> noises = {});
    virtual void postFixedUpdate();
    const ActorState &state() const;
    const Decided &decided() const;
    const Observed &observed() const;
    const PhysicsBody &body() const;
    const NavigationProfile &profile() const;
    std::string_view stateName() const;
    const SheetData &drawnFrom() const;
    std::optional<int> currentNodeId() const;
    std::optional<int> targetNodeId() const;
    glm::vec2 feet() const;
    void standAt(const glm::vec2 &feet);
    const Health &health() const;
    bool alive() const;
    bool takeHit(const Hit &hit);
    virtual std::optional<Hurting> hurting() const;
    bool strike(Actor &target);
    Event<Actor> onHurt, onDeath;
    Event<Actor, const std::string &> onCue;
    Event<Actor, float> onTick;
    Event<Actor, const Noise &> onNoise;

    const FactsData &facts() const;
    const FadingFacts &saidLately() const;
    const Asked &fact(const std::string &name) const;
    void fact(const std::string &name, const Asked &value);
    void event(const std::string &name, const Asked &value);
    std::optional<glm::vec2> threatFeet() const;
    float distanceTo(glm::vec2 at) const;
    bool onSameSurfaceAs(glm::vec2 at) const;
    bool corneredBy(glm::vec2 at) const;
    bool onGround() const;
    void walks(const NavigationGraph &navigationGraph);

protected:
    explicit Actor(const ActorData &data);
    void setBehavior(std::unique_ptr<ActorBehavior> newBehavior);
    void declare(const FactsData &facts);
    virtual void hurt();
    virtual void died();
    ActorBehaviorContext behaviorContext(const NavigationGraph &navigationGraph) const;

private:
    std::optional<AABB> swingBox() const;
    const NavigationGraph &graphWalked() const;
    void say(const std::string &name, const Asked &value);
    void forgetTheTick();
    float howFarItFell();
    FactsData declared;
    FadingFacts lately;
    FactsData known;
    std::vector<std::string> saidForTheTick;
    const NavigationGraph *walking = nullptr;
    float highestSinceTheGround = std::numeric_limits<float>::max();
    std::optional<glm::vec2> threat;
    Abilities abilities;
    Decided decisions;
    Observed observations;
    PhysicsBody physicsBody;
    Animator animator;
    ActorState actorState;
    SheetData sheet;
    NavigationProfile navigationProfile;
    Health hp;
    std::unique_ptr<ActorBehavior> behavior;
};
