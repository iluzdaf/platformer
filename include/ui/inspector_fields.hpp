#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"
#include "tile_map/tile_index.hpp"

inspector::Edited drawCustomField(std::string_view name, TileIndex &value);

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
