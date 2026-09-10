#pragma once

#include <optional>

#include <string>
#include "ui/saveable.hpp"

struct EditorCommands;
struct GameData;
class TextureCache;

class GameSettingsUi
{
public:
    void draw(GameData &gameData, const TextureCache &textures, EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    std::optional<std::string> cannotSaveBecause(const GameData &gameData) const;
    bool reloaded(GameData &current, const GameData &onDisk);

private:
    Saveable saveable;
    std::string askedToWarm;
};
