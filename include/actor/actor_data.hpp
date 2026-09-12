#pragma once

#include <optional>
#include <glm/gtc/matrix_transform.hpp>
#include "serialization/glm_vec2_meta.hpp" // IWYU pragma: keep
#include "assets/sheet_data.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "animations/animator_data.hpp"
#include "physics/physics_body_data.hpp"
#include "combat/health_data.hpp"

struct ActorData
{
    SheetData sheet;
    std::optional<glm::vec2> size;
    PhysicsBodyData physicsBodyData;
    AbilitiesData abilities;
    std::optional<AnimatorData> animationData;
    HealthData healthData;
    float fallFromHeightThreshold = 180;
};

inline glm::vec2 drawnSizeOf(const ActorData &actorData)
{
    return actorData.size.value_or(glm::vec2(actorData.sheet.cellSize));
}
