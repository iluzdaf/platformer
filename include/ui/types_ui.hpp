#pragma once

#include <optional>
#include <string>
#include "ui/saveable.hpp"
#include "ui/type_shown.hpp"
#include "ui/renaming.hpp"

struct GameData;
class TextureCache;
struct EditorCommands;

class TypesUi
{
public:
    void draw(GameData &gameData, const TextureCache &textures, EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    std::optional<std::string> cannotSaveBecause(const GameData &gameData) const;
    void reloaded(GameData &current, const GameData &onDisk);
    void show(const TypeShown &type);

private:
    void drawChooser(GameData &gameData);
    void drawRename(const GameData &gameData);
    void drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands);

    Saveable saveable;
    Renaming npcRenaming, pickupRenaming;
    TypeShown showing;
    std::string askedToWarm;
};
