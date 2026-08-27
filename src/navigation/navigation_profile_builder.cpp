#include "navigation/navigation_profile_builder.hpp"
#include "navigation/motion_arcs.hpp"
#include "actor/actor_data.hpp"

NavigationProfile buildNavigationProfile(const ActorData &actorData)
{
    return NavigationProfile{
        actorData.physicsBodyData.colliderSize,
        simulateJumpArcs(actorData.motionData),
        actorData.motionData.gravityAbilityData.has_value()};
}
