#include <string_view>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/score_icon_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/data_inspector.hpp"
#include "ui/frame_picker.hpp"
#include "game/score_icon_data.hpp"

inspector::Edited drawCustomField(std::string_view name, ScoreIconData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited = inspector::draw("sheet", value.sheet);
    edited |= drawFrameField("frame", value.frame);

    ImGui::TreePop();
    return edited;
}
