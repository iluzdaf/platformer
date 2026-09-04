#include <algorithm>
#include <cfloat>
#include <string>
#include <string_view>
#include <vector>
#include <glm/glm.hpp>
#include <imgui.h>
#include "ui/sheet_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include "ui/unsaved_colours.hpp"
#include "assets/asset_paths.hpp"
#include "assets/sheet_data.hpp"

namespace
{
    inspector::Edited drawTextureChooser(std::string &texture)
    {
        std::vector<std::string> offered = assets::filesIn(assets::Textures, ".png");
        bool picked = false;

        ImGui::TextUnformatted("texture");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo("##texture", texture.empty() ? "none" : texture.c_str()))
        {
            for (const std::string &path : offered)
                if (ImGui::Selectable(path.c_str(), path == texture))
                {
                    picked = texture != path;
                    texture = path;
                }

            ImGui::EndCombo();
        }

        if (texture.empty())
            ImGui::TextColored(CannotSaveColour, "names no sheet to draw from");
        else if (std::find(offered.begin(), offered.end(), texture) == offered.end())
            ImGui::TextColored(CannotSaveColour, "no such file under textures");

        return {picked, picked};
    }
}

inspector::Edited drawSheetFields(SheetData &value)
{
    inspector::Edited edited = drawTextureChooser(value.texture);
    edited |= inspector::draw("cellSize", value.cellSize);

    return edited;
}

inspector::Edited drawSquareSheetFields(SheetData &value)
{
    inspector::Edited edited = drawTextureChooser(value.texture);

    int side = value.cellSize.x;
    inspector::Edited squared = inspector::draw("cellSize", side);
    if (squared)
        value.cellSize = glm::ivec2(side);

    if (value.cellSize.x != value.cellSize.y)
        ImGui::TextColored(
            CannotSaveColour,
            "cells %d by %d, and a tile map lays out squares",
            value.cellSize.x,
            value.cellSize.y);

    return edited |= squared;
}

inspector::Edited drawCustomField(std::string_view name, SheetData &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited = drawSheetFields(value);

    ImGui::TreePop();
    return edited;
}
