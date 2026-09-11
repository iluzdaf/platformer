#pragma once

#include "ui/last_answer.hpp"

#include <functional>
#include <optional>
#include <string>
#include <vector>
#include "assets/asset_paths.hpp"
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "game/levels_data.hpp"
#include "ui/renaming.hpp"
#include "ui/saveable.hpp"

class Level;
struct EditorCommands;

struct LevelsAsked
{
    std::optional<std::string> made;
};

class LevelsUi
{
public:
    using WriteLevels = std::function<void(const LevelsData &)>;

    explicit LevelsUi(
        std::string levelsDirectory = std::string(assets::Levels),
        WriteLevels writeLevels = saveLevels);

    LevelsAsked draw(
        LevelsData &levels,
        const LevelData &playing,
        const std::string &levelPath,
        int tileSize,
        EditorCommands &commands,
        bool levelHasUnsavedChanges);
    std::string add(const LevelData &playing, int tileSize, EditorCommands &commands);
    void remove(const std::string &levelPath, EditorCommands &commands);

    bool save(LevelsData &levels, LevelData &playing);
    std::string revert(LevelsData &levels, const std::string &levelPath);
    bool unsavedSince(const LevelsData &levels);
    std::optional<std::string> cannotSaveBecause(const LevelsData &levels);
    void reloaded(LevelsData &current, const LevelsData &onDisk);

    std::vector<std::string> offered() const;

private:
    std::optional<std::string> firstRemainingAfter(const std::string &leaving) const;

    std::string levelsDirectory;
    WriteLevels writeLevels;
    Saveable saveable;
    LastAnswer firstLevelGate;
    Renaming renaming;
    std::optional<std::string> askedToSwitchTo;
    std::vector<std::string> made;
};
