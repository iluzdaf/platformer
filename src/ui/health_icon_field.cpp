#include <string_view>
#include <imgui.h>
#include "ui/data_inspector.hpp"
#include "ui/frame_picker.hpp"
#include "ui/health_icon_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/marked_label.hpp"
#include "game/health_icon_data.hpp"

inspector::Edited drawCustomField(std::string_view name, HealthIconData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited = inspector::draw("sheet", value.sheet);
    edited |= drawFrameField("full", value.full);
    edited |= drawFrameField("spent", value.spent);

    ImGui::TreePop();
    return edited;
}
