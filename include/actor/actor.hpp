#pragma once

#include <memory>
#include <optional>
#include <string_view>
#include "actor/actor_state.hpp"
#include "actor/actor_data.hpp"
#include "actor/actor_motion.hpp"
#include "actor/actor_behavior.hpp"
#include "animations/animation_manager.hpp"
#include "physics/physics_body.hpp"
#include "navigation/navigation_profile.hpp"
#include "actor/actor_behavior_context.hpp"

class TileMap;
class Level;
class NavigationGraph;

class Actor
{
public:
    virtual ~Actor() = default;
    void preFixedUpdate();
    void fixedUpdate(
        float deltaTime,
        const Level &level,
        std::optional<glm::vec2> threatPosition = std::nullopt);
    virtual void postFixedUpdate();
    const ActorState &getState() const;
    const ActorMotion &getMotion() const;
    const PhysicsBody &getPhysicsBody() const;
    const glm::vec2 &getPosition() const;
    const NavigationProfile &getNavigationProfile() const;
    std::string_view getStateName() const;
    void setPosition(const glm::vec2 &position);

protected:
    explicit Actor(const ActorData &data);
    void setBehavior(std::unique_ptr<ActorBehavior> newBehavior);
    ActorBehaviorContext behaviorContext(
        const NavigationGraph &navigationGraph,
        std::optional<glm::vec2> threatPosition) const;

    ActorMotion motion;
    PhysicsBody physicsBody;
    AnimationManager animationManager;
    ActorState actorState;
    NavigationProfile navigationProfile;
    std::unique_ptr<ActorBehavior> behavior;
};
