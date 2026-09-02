#include <optional>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "ui/tile_field_context.hpp"
#include "ui/tile_picker.hpp"
#include "tile_map/tile_index.hpp"

inspector::Edited drawCustomField(std::string_view name, TileIndex &value)
{
    const TileFieldContext *offering = tilesOnOffer();
    if (!offering || !offering->sheet)
        return inspector::drawNamed(name, value.value);

    ImGui::TextUnformatted(name.data(), name.data() + name.size());
    ImGui::SameLine();

    std::string popup = std::string("##tiles") + std::string(name);
    if (drawTileCell(*offering->sheet, offering->tileSet.tileSize, value.value))
        ImGui::OpenPopup(popup.c_str());

    inspector::Edited edited;
    if (ImGui::BeginPopup(popup.c_str()))
    {
        ImGui::BeginChild("picking", ImVec2(260.0f, 200.0f));
        std::optional<int> picked =
            drawTilePicker(*offering->sheet, offering->tileSet, value.value);
        ImGui::EndChild();

        if (picked && *picked != value.value)
        {
            value.value = *picked;
            edited = {true, true};
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return edited;
}
