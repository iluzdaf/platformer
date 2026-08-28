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
        std::string now = asJson(value);
        std::string &saved = asSaved[std::string(name)];
        if (saved.empty())
            saved = now;

        if (saved == now)
        {
            ImGui::TextDisabled("saved");
            return false;
        }

        if (ImGui::Button("save"))
        {
            save(value);
            saved = asJson(value);
        }

        ImGui::SameLine();
        if (ImGui::Button("revert"))
        {
            std::ignore = glz::read_json(value, saved);
            return true;
        }

        ImGui::SameLine();
        ImGui::TextDisabled("unsaved");

        return false;
    }

    void forget()
    {
        asSaved.clear();
    }

private:
    std::map<std::string, std::string> asSaved;

    template <class T> static std::string asJson(const T &value)
    {
        std::string json;
        if (glz::write_json(value, json))
            throw std::runtime_error("Failed to serialise for comparison");

        return json;
    }
};
