#include <algorithm>
#include <cfloat>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/file_chooser.hpp"
#include "ui/inspector_edited.hpp"
#include <optional>
#include "assets/asset_paths.hpp"

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    std::string_view extension)
{
    return drawFileChooser(label, path, folder, assets::filesIn(folder, extension));
}

inspector::Edited drawFileChooser(
    std::string_view label,
    std::string &path,
    std::string_view folder,
    const std::vector<std::string> &offered)
{
    std::string where(folder);
    std::optional<std::string> cannot;
    if (path.empty())
        cannot = "names no file under " + where;
    else if (std::find(offered.begin(), offered.end(), path) == offered.end())
        cannot = "no such file under " + where;

    bool picked = false;

    inspector::drawLabel(label, inspector::markedHere(), cannot.has_value());
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

    if (cannot)
        inspector::drawRefusal(*cannot);

    return {picked, picked};
}
