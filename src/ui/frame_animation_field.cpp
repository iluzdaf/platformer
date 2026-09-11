#include <cstddef>
#include <optional>
#include <vector>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/frame_picker.hpp"
#include "animations/frame_animation_data.hpp"

namespace
{
    inspector::Edited drawFrames(const SheetInScope *offering, std::vector<int> &frames)
    {
        inspector::InField here("frames");
        const bool changed = inspector::changedHere(frames);
        inspector::Marking marking(changed);
        if (!inspector::drawFold("frames", changed))
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
                edited |= inspector::draw(std::to_string(at), frames[at]);

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

    inspector::Edited drawCues(std::vector<FrameCueData> &cues, std::size_t frameCount)
    {
        inspector::InField here("cues");
        const bool changed = inspector::changedHere(cues);
        inspector::Marking marking(changed);
        if (!inspector::drawFold("cues", changed))
            return {};

        inspector::Edited edited;
        std::optional<std::size_t> takeAway;
        for (std::size_t at = 0; at < cues.size(); ++at)
        {
            ImGui::PushID(static_cast<int>(at));
            if (ImGui::SmallButton("-"))
                takeAway = at;

            ImGui::SameLine();
            inspector::InField cue(std::to_string(at));
            edited |= inspector::draw("frame", cues[at].frame);
            edited |= inspector::draw("name", cues[at].name);
            ImGui::PopID();
        }

        bool addAsked = ImGui::SmallButton("+");
        if (takeAway)
            cues.erase(cues.begin() + static_cast<std::ptrdiff_t>(*takeAway));
        else if (addAsked)
            cues.push_back(
                FrameCueData{frameCount == 0 ? 0 : static_cast<int>(frameCount) - 1, ""});

        if (takeAway || addAsked)
            edited |= inspector::Edited{true, true};

        ImGui::TreePop();
        return edited;
    }
}

inspector::Edited drawCustomField(std::string_view name, FrameAnimationData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited = drawFrames(sheetInScope(), value.frames);
    edited |= inspector::draw("frameDuration", value.frameDuration);
    if (value.frameDuration <= 0.0f)
        value.frameDuration = 0.01f;
    edited |= inspector::draw("loops", value.loops);
    edited |= drawCues(value.cues, value.frames.size());

    ImGui::TreePop();
    return edited;
}
