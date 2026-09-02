#include <cstddef>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/tile_animation_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "ui/tile_field_context.hpp"
#include "ui/tile_picker.hpp"
#include "animations/frame_animation_data.hpp"
#include "animations/tile_animation_data.hpp"

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill)
{
    if (animation.frames.empty() || animation.frameDuration <= 0.0f)
        return whenStill;

    auto step = static_cast<std::size_t>(seconds / animation.frameDuration);

    return animation.frames[step % animation.frames.size()];
}

inspector::Edited drawCustomField(std::string_view name, TileAnimationData &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited = inspector::drawFields(value.frameAnimationData);

    if (value.frameAnimationData.frameDuration <= 0.0f)
        value.frameAnimationData.frameDuration = 0.01f;

    const TileFieldContext *offering = tilesOnOffer();
    if (offering && offering->sheet)
        drawTileImage(
            *offering->sheet,
            offering->tileSet.tileSize,
            previewFrameAt(value.frameAnimationData, ImGui::GetTime(), offering->tileIndex),
            AnimationPreviewSize);

    ImGui::TreePop();
    return edited;
}
