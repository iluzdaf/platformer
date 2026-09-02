#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"
#include "tile_map/tile_index.hpp"

struct TileAnimationData;
struct TileColliderData;

inspector::Edited drawCustomField(std::string_view name, TileIndex &value);
inspector::Edited drawCustomField(std::string_view name, TileAnimationData &value);
inspector::Edited drawCustomField(std::string_view name, TileColliderData &value);

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
