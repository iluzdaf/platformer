#include <string_view>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/data_inspector.hpp"
#include "tile_map/tile_index.hpp"

inspector::Edited drawCustomField(std::string_view name, TileIndex &value)
{
    return inspector::drawNamed(name, value.value);
}
