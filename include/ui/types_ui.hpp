#pragma once

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>
#include "ui/saveable.hpp"
#include "ui/last_answer.hpp"
#include "ui/type_shown.hpp"
#include "ui/renaming.hpp"
#include "ui/state_machine_shown.hpp"
#include "assets/asset_paths.hpp"
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "player/player_data.hpp"

struct LevelData;
class Level;
class TextureCache;
struct EditorCommands;
struct SheetInScope;
struct AnimatorData;
struct ActorData;

class TypesUi
{
public:
    using WriteNpcs = std::function<void(const std::map<std::string, NpcData> &)>;
    using WritePickups = std::function<void(const std::map<std::string, PickupData> &)>;
    using WritePlayer = std::function<void(const PlayerData &)>;

    explicit TypesUi(
        std::string levelsDirectory = std::string(assets::Levels),
        WriteNpcs writeNpcs = saveNpcData,
        WritePickups writePickups = savePickupData,
        WritePlayer writePlayer = savePlayerData);

    void draw(
        GameData &gameData,
        const TextureCache &textures,
        EditorCommands &commands,
        const Level *live = nullptr);
    bool save(GameData &gameData, LevelData &playing);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    std::optional<std::string> cannotSaveBecause(const GameData &gameData);
    bool reloaded(GameData &current, const GameData &onDisk);
    void show(const TypeShown &type);
    std::vector<TypeShown> offered(const GameData &gameData) const;
    void add(GameData &gameData, TypeShown::What what);
    void remove(GameData &gameData);
    bool namesChanged();

private:
    void drawChooser(GameData &gameData);
    void drawRename(const GameData &gameData);
    void drawShown(
        GameData &gameData,
        const TextureCache &textures,
        EditorCommands &commands,
        const Level *live);
    void drawActorPreview(const SheetInScope &scope, const ActorData &actorData);

    std::string levelsDirectory;
    WriteNpcs writeNpcs;
    WritePickups writePickups;
    WritePlayer writePlayer;
    Saveable saveable;
    LastAnswer castGate;
    Renaming npcRenaming, pickupRenaming;
    TypeShown showing = thePlayer();
    bool namesTouched = false;
    std::string previewing = "idle";
    MachineShown machineShown;
    std::string askedToWarm;
};
