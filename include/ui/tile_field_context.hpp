#pragma once

#include "assets/sheet.hpp"

class Texture2D;

struct TileFieldContext
{
    const Texture2D *sheet = nullptr;
    Sheet tileSet;
    int tileIndex = 0;
};

const TileFieldContext *tilesOnOffer();

class ShowingTilesFrom
{
public:
    explicit ShowingTilesFrom(const TileFieldContext &context);
    ~ShowingTilesFrom();

    ShowingTilesFrom(const ShowingTilesFrom &) = delete;
    ShowingTilesFrom &operator=(const ShowingTilesFrom &) = delete;

private:
    const TileFieldContext *before = nullptr;
};
