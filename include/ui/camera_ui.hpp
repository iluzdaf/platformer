#pragma once

#include <string>

#include <optional>

#include "ui/saveable.hpp"

struct EditorCommands;
struct GameData;
class Camera2D;

class CameraUi
{
public:
    void draw(GameData &gameData, const Camera2D &camera, EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    std::optional<std::string> cannotSaveBecause(const GameData &gameData) const;
    bool reloaded(GameData &current, const GameData &onDisk);

private:
    Saveable saveable;
};
