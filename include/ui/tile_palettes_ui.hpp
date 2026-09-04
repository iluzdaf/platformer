#pragma once

#include <functional>
#include <optional>
#include <string>
#include "ui/armed.hpp"
#include "ui/saveable.hpp"
#include "ui/renaming.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "assets/asset_paths.hpp"
#include "game/game_data.hpp"

class TextureCache;
struct EditorCommands;

class TilePalettesUi
{
public:
    using WritePalettes = std::function<void(const TilePalettes &)>;

    explicit TilePalettesUi(
        std::string levelsDirectory = std::string(assets::Levels),
        WritePalettes writePalettes = saveTilePalettes);

    void draw(
        TilePalettes &tilePalettes,
        const TextureCache &textures,
        EditorCommands &commands,
        std::optional<Armed> &armed);

    void show(const std::string &palette);
    const std::string &shownPalette() const;
    void add(TilePalettes &tilePalettes);
    void remove(TilePalettes &tilePalettes);

    void save(TilePalettes &tilePalettes);
    void revert(TilePalettes &tilePalettes);
    bool unsavedSince(const TilePalettes &tilePalettes);
    std::optional<std::string> cannotSaveBecause() const;
    void reloaded(TilePalettes &current, const TilePalettes &onDisk);

private:
    void drawChooser(TilePalettes &tilePalettes);
    void drawRename(const TilePalettes &tilePalettes);
    bool shownIn(const TilePalettes &tilePalettes, const std::string &name) const;
    std::string firstShownIn(const TilePalettes &tilePalettes) const;

    std::string levelsDirectory;
    WritePalettes writePalettes;
    Saveable saveable;
    Renaming renaming;
    std::string selectedPalette, askedToWarm;
};

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes);
