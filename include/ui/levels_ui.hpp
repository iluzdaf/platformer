#pragma once

#include <optional>
#include <string>
#include "ui/saveable.hpp"

class Level;
class Levels;
struct EditorCommands;

class LevelsUi
{
public:
    void draw(
        Levels &levels,
        const Level &level,
        EditorCommands &commands,
        bool levelHasUnsavedChanges);
    void save(Levels &levels);
    bool hasUnsavedChanges(const Levels &levels) const;
    void valuesReplaced();

private:
    Saveable saveable;
    std::optional<std::string> askedToSwitchTo;
};
