#include <cfloat>
#include <string>
#include <string_view>
#include <glm/glm.hpp>
#include <imgui.h>
#include "ui/sheet_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include "ui/file_chooser.hpp"
#include "ui/unsaved_colours.hpp"
#include "assets/asset_paths.hpp"
#include "assets/sheet_data.hpp"

inspector::Edited drawSheetFields(SheetData &value)
{
    inspector::Edited edited =
        drawFileChooser("texture", value.texture.path, assets::Textures, ".png");
    edited |= inspector::draw("cellSize", value.cellSize);

    return edited;
}

inspector::Edited drawSquareSheetFields(SheetData &value)
{
    inspector::Edited edited =
        drawFileChooser("texture", value.texture.path, assets::Textures, ".png");

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
