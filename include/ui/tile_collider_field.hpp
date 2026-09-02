#pragma once

#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

inline constexpr float ColliderPreviewSize = 96.0f;

std::pair<ImVec2, ImVec2> colliderRect(
    ImVec2 tileAt,
    float scale,
    glm::vec2 offset,
    glm::vec2 size);
