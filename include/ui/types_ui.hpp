#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include "ui/saveable.hpp"
#include "ui/type_shown.hpp"
#include "ui/renaming.hpp"
#include "assets/asset_paths.hpp"
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"

struct LevelData;
class TextureCache;
struct EditorCommands;
struct SheetInScope;
struct ActorAnimationData;

class TypesUi
{
public:
    using WriteNpcs = std::function<void(const std::map<std::string, NpcData> &)>;
    using WritePickups = std::function<void(const std::map<std::string, PickupData> &)>;

    explicit TypesUi(
        std::string levelsDirectory = std::string(assets::Levels),
        WriteNpcs writeNpcs = saveNpcData,
        WritePickups writePickups = savePickupData);

    void draw(GameData &gameData, const TextureCache &textures, EditorCommands &commands);
    bool save(GameData &gameData, LevelData &playing);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    std::optional<std::string> cannotSaveBecause(const GameData &gameData) const;
    void reloaded(GameData &current, const GameData &onDisk);
    void show(const TypeShown &type);

private:
    void drawChooser(GameData &gameData);
    void drawRename(const GameData &gameData);
    void drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands);
    void drawActorPreview(const SheetInScope &scope, const ActorAnimationData &animations);

    std::string levelsDirectory;
    WriteNpcs writeNpcs;
    WritePickups writePickups;
    Saveable saveable;
    Renaming npcRenaming, pickupRenaming;
    TypeShown showing;
    std::string previewing = "idle";
    std::string askedToWarm;
};
