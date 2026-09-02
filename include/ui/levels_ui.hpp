#pragma once

#include <optional>
#include <string>
#include "ui/saveable.hpp"

class Level;
class Levels;
struct EditorCommands;

struct SwitchingLevel
{
    std::optional<std::string> loadNow;
    std::optional<std::string> waitingOn;

    bool operator==(const SwitchingLevel &) const = default;
};

SwitchingLevel switching(
    const std::optional<std::string> &chosen,
    bool levelHasUnsavedChanges,
    const std::optional<std::string> &waitingOn,
    bool switchPressed,
    bool cancelPressed);

class LevelsUi
{
public:
    void draw(
        Levels &levels,
        const Level &level,
        EditorCommands &commands,
        bool levelHasUnsavedChanges);
    bool hasUnsavedChanges(const Levels &levels) const;
    void valuesReplaced();

private:
    Saveable saveable;
    std::optional<std::string> askedToSwitchTo;
};
