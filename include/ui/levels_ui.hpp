#pragma once

#include <optional>
#include <string>
#include "ui/saveable.hpp"
#include "game/levels_data.hpp"

class Level;
struct EditorCommands;

class LevelsUi
{
public:
    void draw(
        LevelsData &levels,
        const std::string &levelPath,
        EditorCommands &commands,
        bool levelHasUnsavedChanges);
    void save(const LevelsData &levels);
    void revert(LevelsData &levels);
    bool unsavedSince(const LevelsData &levels);
    void reloaded(LevelsData &current, const LevelsData &onDisk);

private:
    Saveable saveable;
    std::optional<std::string> askedToSwitchTo;
};
