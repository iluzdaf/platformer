#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include <signals.hpp>
#include "actor/actor_state.hpp"
#include "assets/sheet_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"
#include "actor/actor_behavior.hpp"
#include "animations/animator.hpp"
#include "physics/physics_body.hpp"
#include "navigation/navigation_profile.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/health.hpp"

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
        std::optional<glm::vec2> threatFeet = std::nullopt);
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
    fteng::signal<void()> onHurt, onDeath;

protected:
    explicit Actor(const ActorData &data);
    void setBehavior(std::unique_ptr<ActorBehavior> newBehavior);
    virtual void hurt();
    virtual void died();
    ActorBehaviorContext behaviorContext(
        const NavigationGraph &navigationGraph,
        std::optional<glm::vec2> threatFeet) const;

private:
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
