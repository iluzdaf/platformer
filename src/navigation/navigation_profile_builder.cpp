#include "navigation/navigation_profile_builder.hpp"
#include "navigation/jump_simulation.hpp"
#include "actor/actor_data.hpp"
#include "navigation/navigation_profile.hpp"

NavigationProfile buildNavigationProfile(const ActorData &actorData)
{
    return NavigationProfile{
        simulateJumpArcs(actorData.motionData), actorData.motionData, actorData.physicsBodyData};
}
