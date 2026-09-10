#pragma once

#include "tile_map/tile_collider_data.hpp"

#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

struct ActorAnimationData;
struct FrameAnimationData;
struct SheetInScope;
struct TileData;

inline constexpr float PreviewSize = 96.0f;

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill);

std::pair<ImVec2, ImVec2> colliderRect(
    ImVec2 tileAt,
    float scale,
    glm::vec2 offset,
    glm::vec2 size);

struct NamedAnimation
{
    std::string name;
    const FrameAnimationData *animation;
};

std::vector<NamedAnimation> animationsOf(const ActorAnimationData &animations);

const NamedAnimation &animationNamed(
    const std::vector<NamedAnimation> &offered,
    std::string_view name);

ImVec2 drawAnimationPreview(const SheetInScope &offering, const FrameAnimationData &animation);

void drawColliderOver(ImVec2 at, float scale, glm::vec2 offset, glm::vec2 size);

TileColliderData colliderShownFor(const TileData &tile, int tileSize);

float tilePreviewScale(int tileSize);

void drawTilePreview(
    const SheetInScope &offering,
    int tileIndex,
    const TileData &tile,
    int tileSize);
