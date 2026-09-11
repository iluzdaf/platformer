#include <optional>
#include <string>
#include <string_view>
#include <imgui.h>
#include <string_view>
#include "ui/data_inspector.hpp"
#include "ui/frame_picker.hpp"
#include "ui/marked_label.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/tile_picker.hpp"

inspector::Edited drawFramePicked(const SheetInScope &offering, int &frame)
{
    std::string popup = "##pick" + std::to_string(frame);
    if (drawTileCell(*offering.texture, offering.sheet.cellSize.x, frame))
        ImGui::OpenPopup(popup.c_str());

    inspector::Edited edited;
    if (ImGui::BeginPopup(popup.c_str()))
    {
        ImGui::BeginChild("picking", ImVec2(PickerWidth, PickerHeight));
        std::optional<int> picked = drawTilePicker(*offering.texture, offering.sheet, frame);
        ImGui::EndChild();

        if (picked && *picked != frame)
        {
            frame = *picked;
            edited = {true, true};
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return edited;
}

inspector::Edited drawFrameField(std::string_view name, int &frame)
{
    const SheetInScope *offering = sheetInScope();
    if (!offering || !offering->texture)
        return inspector::draw(name, frame);

    inspector::InField here(name);
    const bool changed = inspector::changedHere(frame);
    inspector::Marking marking(changed);
    inspector::drawLabel(name, changed);
    ImGui::SameLine();

    ImGui::PushID(std::string(name).c_str());
    inspector::Edited edited = drawFramePicked(*offering, frame);
    ImGui::PopID();

    return edited;
}
