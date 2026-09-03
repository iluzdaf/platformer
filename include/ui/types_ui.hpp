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
    std::optional<TypeRenamed> draw(
        GameData &gameData,
        const TextureCache &textures,
        EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    void valuesReplaced();
    void show(const TypeShown &type);

private:
    void drawChooser(GameData &gameData);
    std::optional<TypeRenamed> drawRename(GameData &gameData);
    void drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands);

    Saveable saveable;
    Renaming npcRenaming, pickupRenaming;
    TypeShown showing;
    std::string askedToWarm;
};
