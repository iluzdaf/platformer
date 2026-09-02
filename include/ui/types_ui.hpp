#pragma once

#include <string>
#include "ui/saveable.hpp"
#include "ui/type_shown.hpp"

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
    void valuesReplaced();
    void show(const TypeShown &type);

private:
    void drawChooser(GameData &gameData);
    void drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands);

    Saveable saveable;
    TypeShown showing;
    std::string askedToWarm;
};
