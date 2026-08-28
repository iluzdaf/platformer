#include "navigation/navigation_profile_builder.hpp"
#include "navigation/jump_simulation.hpp"
#include "actor/actor_data.hpp"

NavigationProfile buildNavigationProfile(const ActorData &actorData)
{
    return NavigationProfile{
        actorData.physicsBodyData.colliderSize,
        simulateJumpArcs(actorData.motionData),
        actorData.motionData,
        actorData.physicsBodyData};
}
