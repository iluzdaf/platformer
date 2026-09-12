#include <cfloat>
#include <string_view>
#include <glm/glm.hpp>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/sheet_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include <string>
#include "assets/sheet_data.hpp"

inspector::Edited drawSheetFields(SheetData &value)
{
    inspector::Edited edited = inspector::draw("texture", value.texture);
    edited |= inspector::draw("cellSize", value.cellSize);

    return edited;
}

inspector::Edited drawSquareSheetFields(SheetData &value)
{
    inspector::Edited edited = inspector::draw("texture", value.texture);

    int side = value.cellSize.x;
    const bool notSquare = value.cellSize.x != value.cellSize.y;
    inspector::Edited squared = inspector::drawAs("cellSize", side, value.cellSize, notSquare);
    if (squared)
        value.cellSize = glm::ivec2(side);

    if (notSquare)
        inspector::drawRefusal(
            "cells " + std::to_string(value.cellSize.x) + " by " +
            std::to_string(value.cellSize.y) + ", and a tile map lays out squares");

    return edited |= squared;
}

inspector::Edited drawCustomField(std::string_view name, SheetData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited = drawSheetFields(value);

    ImGui::TreePop();
    return edited;
}
