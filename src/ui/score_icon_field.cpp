#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/score_icon_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include "ui/frame_picker.hpp"
#include "ui/sheet_in_scope.hpp"
#include "game/score_icon.hpp"

inspector::Edited drawCustomField(std::string_view name, ScoreIcon &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited = inspector::draw("sheet", value.sheet);

    const SheetInScope *offering = sheetInScope();
    if (offering && offering->texture)
    {
        ImGui::TextUnformatted("frame");
        ImGui::SameLine();
        edited |= drawFramePicked(*offering, value.frame);
    }
    else
        edited |= inspector::drawNamed("frame", value.frame);

    ImGui::TreePop();
    return edited;
}
