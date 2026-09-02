#pragma once

#include <glaze/glaze.hpp>
#include "tile_map/tile_index.hpp"

template <> struct glz::meta<TileIndex>
{
    using T = TileIndex;
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = &T::value;
};
