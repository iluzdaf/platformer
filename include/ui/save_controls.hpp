#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <imgui.h>
#include <glaze/glaze.hpp>
#include "ui/saveable.hpp"

inline constexpr ImVec4 UnsavedColour{1.0f, 1.0f, 0.4f, 1.0f};

inline constexpr ImVec4 CannotSaveColour{1.0f, 0.4f, 0.4f, 1.0f};

template <class Save, class Revert>
bool drawSaveControls(
    Saveable &saveable,
    std::string_view name,
    std::string now,
    Save &&save,
    Revert &&revert,
    const std::optional<std::string> &cannotSaveBecause = std::nullopt)
{
    saveable.seen(name, now);

    if (!saveable.unsaved(name, now))
    {
        ImGui::TextDisabled("saved");
        return false;
    }

    ImGui::BeginDisabled(cannotSaveBecause.has_value());
    bool saveAsked = ImGui::Button("save");
    ImGui::EndDisabled();

    if (saveAsked && !cannotSaveBecause)
    {
        save();
        saveable.saved(name, std::move(now));
        return false;
    }

    ImGui::SameLine();
    if (ImGui::Button("revert"))
    {
        revert(saveable.lastSeen(name));
        return true;
    }

    ImGui::SameLine();
    ImGui::TextDisabled("unsaved");

    if (cannotSaveBecause)
        ImGui::TextColored(CannotSaveColour, "%s", cannotSaveBecause->c_str());

    return false;
}

template <class T, class Save>
bool drawSaveControls(
    Saveable &saveable,
    std::string_view name,
    T &value,
    Save &&save,
    const std::optional<std::string> &cannotSaveBecause = std::nullopt)
{
    return drawSaveControls(
        saveable,
        name,
        asJson(value),
        [&] { save(value); },
        [&](const std::string &lastSeen) { std::ignore = glz::read_json(value, lastSeen); },
        cannotSaveBecause);
}
