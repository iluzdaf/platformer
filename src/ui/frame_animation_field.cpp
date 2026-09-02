#include <cstddef>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/frame_animation_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/tile_picker.hpp"
#include "ui/frame_picker.hpp"
#include "animations/frame_animation_data.hpp"

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill)
{
    if (animation.frames.empty() || animation.frameDuration <= 0.0f)
        return whenStill;

    auto step = static_cast<std::size_t>(seconds / animation.frameDuration);

    return animation.frames[step % animation.frames.size()];
}

namespace
{
    inspector::Edited drawFrames(const SheetInScope *offering, std::vector<int> &frames)
    {
        if (!ImGui::TreeNode("frames"))
            return {};

        inspector::Edited edited;
        std::optional<std::size_t> takeAway;
        for (std::size_t at = 0; at < frames.size(); ++at)
        {
            ImGui::PushID(static_cast<int>(at));
            if (ImGui::SmallButton("-"))
                takeAway = at;

            ImGui::SameLine();
            if (offering && offering->texture)
                edited |= drawFramePicked(*offering, frames[at]);
            else
                edited |= inspector::drawNamed(std::to_string(at), frames[at]);

            ImGui::PopID();
        }

        bool addAsked = ImGui::SmallButton("+");
        if (takeAway)
            frames.erase(frames.begin() + static_cast<std::ptrdiff_t>(*takeAway));
        else if (addAsked)
            frames.push_back(frames.empty() ? 0 : frames.back());

        if (takeAway || addAsked)
            edited |= inspector::Edited{true, true};

        ImGui::TreePop();
        return edited;
    }
}

inspector::Edited drawCustomField(std::string_view name, FrameAnimationData &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    const SheetInScope *offering = sheetInScope();

    inspector::Edited edited = drawFrames(offering, value.frames);
    edited |= inspector::drawNamed("frameDuration", value.frameDuration);

    if (value.frameDuration <= 0.0f)
        value.frameDuration = 0.01f;

    if (offering && offering->texture)
        drawTileImage(
            *offering->texture,
            offering->sheet.cellSize.x,
            previewFrameAt(value, ImGui::GetTime(), offering->frame),
            AnimationPreviewSize);

    ImGui::TreePop();
    return edited;
}
