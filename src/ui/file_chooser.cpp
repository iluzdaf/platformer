#include <algorithm>
#include <cfloat>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/file_chooser.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/unsaved_colours.hpp"
#include "assets/asset_paths.hpp"

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    std::string_view extension)
{
    std::vector<std::string> offered = assets::filesIn(folder, extension);
    bool picked = false;

    ImGui::TextUnformatted(label.data(), label.data() + label.size());
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo(
            ("##" + std::string(label)).c_str(), path.empty() ? "none" : path.c_str()))
    {
        for (const std::string &offer : offered)
            if (ImGui::Selectable(offer.c_str(), offer == path))
            {
                picked = path != offer;
                path = offer;
            }

        ImGui::EndCombo();
    }

    std::string where(folder);
    if (path.empty())
        ImGui::TextColored(CannotSaveColour, "names no file under %s", where.c_str());
    else if (std::find(offered.begin(), offered.end(), path) == offered.end())
        ImGui::TextColored(CannotSaveColour, "no such file under %s", where.c_str());

    return {picked, picked};
}
