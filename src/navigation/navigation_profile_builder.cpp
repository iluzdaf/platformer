#include <optional>
#include <stdexcept>
#include <string>
#include "physics/physics_body_data.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "navigation/jump_simulation.hpp"
#include "actor/actor_data.hpp"
#include "navigation/navigation_profile.hpp"

NavigationProfile buildNavigationProfile(const ActorData &actorData)
{
    if (std::optional<std::string> why = whyNotABody(actorData.physicsBodyData))
        throw std::runtime_error("A body " + *why);

    return NavigationProfile{
        simulateJumpArcs(actorData.motionData), actorData.motionData, actorData.physicsBodyData};
}
