#include <optional>
#include <string>
#include <imgui.h>
#include "ui/frame_picker.hpp"
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
