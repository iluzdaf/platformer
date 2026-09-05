#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/sheet_preview.hpp"
#include "ui/sheet_in_scope.hpp"
#include "actor/actor_animation_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "assets/sheet_data.hpp"
#include "rendering/texture2d.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill)
{
    if (animation.frames.empty() || animation.frameDuration <= 0.0f)
        return whenStill;

    auto step = static_cast<std::size_t>(seconds / animation.frameDuration);

    return animation.frames[step % animation.frames.size()];
}

std::pair<ImVec2, ImVec2> colliderRect(ImVec2 tileAt, float scale, glm::vec2 offset, glm::vec2 size)
{
    ImVec2 low(tileAt.x + offset.x * scale, tileAt.y + offset.y * scale);

    return {low, ImVec2(low.x + size.x * scale, low.y + size.y * scale)};
}

std::vector<NamedAnimation> animationsOf(const ActorAnimationData &animations)
{
    std::vector<NamedAnimation> offered{{"idle", &animations.idle}};
    for (const auto &[name, animation] :
         std::initializer_list<std::pair<const char *, const std::optional<FrameAnimationData> *>>{
             {"walk", &animations.walk},
             {"dash", &animations.dash},
             {"jump", &animations.jump},
             {"fall", &animations.fall},
             {"wallSlide", &animations.wallSlide}})
        if (animation->has_value())
            offered.push_back({name, &animation->value()});

    return offered;
}

const NamedAnimation &animationNamed(
    const std::vector<NamedAnimation> &offered,
    std::string_view name)
{
    if (offered.empty())
        throw std::runtime_error("Nothing is offered to preview");

    for (const NamedAnimation &animation : offered)
        if (animation.name == name)
            return animation;

    return offered.front();
}

namespace
{
    ImVec2 drawFrame(const SheetInScope &offering, int frame)
    {
        const Texture2D &texture = *offering.texture;
        glm::ivec2 cell = offering.sheet.cellSize;
        auto [uvStart, uvEnd] = frameUvRangeIn(
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()),
            frame,
            cell.x,
            cell.y,
            false);
        float height = cell.x > 0
                           ? PreviewSize * static_cast<float>(cell.y) / static_cast<float>(cell.x)
                           : PreviewSize;

        ImVec2 at = ImGui::GetCursorScreenPos();
        ImGui::Image(
            (ImTextureID)(intptr_t)texture.getTextureID(),
            ImVec2(PreviewSize, height),
            ImVec2(uvStart.x, uvStart.y),
            ImVec2(uvEnd.x, uvEnd.y));

        return at;
    }
}

void drawAnimationPreview(const SheetInScope &offering, const FrameAnimationData &animation)
{
    if (!offering.texture)
        return;

    drawFrame(offering, previewFrameAt(animation, ImGui::GetTime(), 0));
}

void drawTilePreview(const SheetInScope &offering, int tileIndex, const TileData &tile)
{
    if (!offering.texture)
        return;

    int frame = tile.animationData
                    ? previewFrameAt(*tile.animationData, ImGui::GetTime(), tileIndex)
                    : tileIndex;
    ImVec2 at = drawFrame(offering, frame);

    glm::vec2 cell(offering.sheet.cellSize);
    TileColliderData collider = tile.collider.value_or(TileColliderData{glm::vec2(0.0f), cell});
    float scale = cell.x > 0.0f ? PreviewSize / cell.x : 0.0f;
    auto [low, high] = colliderRect(at, scale, collider.offset, collider.size);
    ImGui::GetWindowDrawList()->AddRectFilled(low, high, IM_COL32(0, 255, 255, 40));
    ImGui::GetWindowDrawList()->AddRect(low, high, IM_COL32(0, 255, 255, 255));
}
