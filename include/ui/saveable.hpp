#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <imgui.h>
#include <glaze/glaze.hpp>

class Saveable
{
public:
    template <class T, class Save> bool drawControls(std::string_view name, T &value, Save &&save)
    {
        return drawControls(
            name,
            asJson(value),
            [&] { save(value); },
            [&](const std::string &lastSeen) { std::ignore = glz::read_json(value, lastSeen); });
    }

    template <class Save, class Revert>
    bool drawControls(std::string_view name, std::string now, Save &&save, Revert &&revert)
    {
        std::string &lastSeen = asLastSeen[std::string(name)];
        if (lastSeen.empty())
            lastSeen = now;

        if (lastSeen == now)
        {
            ImGui::TextDisabled("saved");
            return false;
        }

        if (ImGui::Button("save"))
        {
            save();
            lastSeen = std::move(now);
            return false;
        }

        ImGui::SameLine();
        if (ImGui::Button("revert"))
        {
            revert(lastSeen);
            return true;
        }

        ImGui::SameLine();
        ImGui::TextDisabled("unsaved");

        return false;
    }

    void valuesReplaced()
    {
        asLastSeen.clear();
    }

private:
    std::map<std::string, std::string> asLastSeen;

    template <class T> static std::string asJson(const T &value)
    {
        std::string json;
        if (glz::write_json(value, json))
            throw std::runtime_error("Failed to serialise for comparison");

        return json;
    }
};
