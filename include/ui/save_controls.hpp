#pragma once

#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <imgui.h>
#include <glaze/glaze.hpp>
#include "ui/saveable.hpp"

inline constexpr ImVec4 UnsavedColour{1.0f, 1.0f, 0.4f, 1.0f};

template <class Save, class Revert>
bool drawSaveControls(
    Saveable &saveable,
    std::string_view name,
    std::string now,
    Save &&save,
    Revert &&revert)
{
    saveable.seen(name, now);

    if (!saveable.unsaved(name, now))
    {
        ImGui::TextDisabled("saved");
        return false;
    }

    if (ImGui::Button("save"))
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

    return false;
}

template <class T, class Save>
bool drawSaveControls(Saveable &saveable, std::string_view name, T &value, Save &&save)
{
    return drawSaveControls(
        saveable,
        name,
        asJson(value),
        [&] { save(value); },
        [&](const std::string &lastSeen) { std::ignore = glz::read_json(value, lastSeen); });
}
