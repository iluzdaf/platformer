#include <string>
#include <string_view>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/tile_collider_field.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/tile_picker.hpp"
#include "tile_map/tile_collider_data.hpp"

std::pair<ImVec2, ImVec2> colliderRect(ImVec2 tileAt, float scale, glm::vec2 offset, glm::vec2 size)
{
    ImVec2 low(tileAt.x + offset.x * scale, tileAt.y + offset.y * scale);

    return {low, ImVec2(low.x + size.x * scale, low.y + size.y * scale)};
}

inspector::Edited drawCustomField(std::string_view name, TileColliderData &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited = inspector::drawFields(value);

    const SheetInScope *offering = sheetInScope();
    if (offering && offering->texture && offering->sheet.cellSize.x > 0)
    {
        ImVec2 tileAt = ImGui::GetCursorScreenPos();
        drawTileImage(
            *offering->texture, offering->sheet.cellSize.x, offering->frame, ColliderPreviewSize);

        float scale = ColliderPreviewSize / static_cast<float>(offering->sheet.cellSize.x);
        auto [low, high] = colliderRect(tileAt, scale, value.offset, value.size);
        ImGui::GetWindowDrawList()->AddRectFilled(low, high, IM_COL32(0, 255, 255, 40));
        ImGui::GetWindowDrawList()->AddRect(low, high, IM_COL32(0, 255, 255, 255));
    }

    ImGui::TreePop();
    return edited;
}
