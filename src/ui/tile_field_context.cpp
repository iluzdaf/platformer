#include "ui/tile_field_context.hpp"

namespace
{
    const TileFieldContext *showing = nullptr;
}

const TileFieldContext *tilesOnOffer()
{
    return showing;
}

ShowingTilesFrom::ShowingTilesFrom(const TileFieldContext &context) : before(showing)
{
    showing = &context;
}

ShowingTilesFrom::~ShowingTilesFrom()
{
    showing = before;
}
