#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/sheet_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include "ui/unsaved_colours.hpp"
#include "assets/sheet.hpp"

inspector::Edited drawCustomField(std::string_view name, Sheet &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited = inspector::draw("texture", value.texture);

    if (value.texture.empty())
        ImGui::TextColored(CannotSaveColour, "names no sheet to draw from");

    edited |= inspector::draw("cellSize", value.cellSize);

    ImGui::TreePop();
    return edited;
}
