#pragma once

#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_state.hpp"
#include "actor/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/levels_data.hpp"
#include "helpers/actors.hpp"
#include "helpers/tiles.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "helpers/temporary_levels.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "pickups/pickup_data.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_ui.hpp"

inline constexpr const char *SomeSheet = "textures/somewhere.png";

inline GameData aSmallGame()
{
    GameData gameData;
    gameData.tilePalettes = theOnlyPalette(aPaletteWithASolidTile());
    gameData.playerData = playerDataWithEveryAbility();
    gameData.playerData.actorData.sheet.texture.path = SomeSheet;

    NpcData rat = setupNpcData();
    rat.actorData.sheet.texture.path = SomeSheet;
    rat.actorData.motionData.pounceAbilityData = PounceAbilityData{};
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    rat.stateMachineBehaviorData->states.push_back(pouncing);
    NpcData spider = setupNpcData();
    spider.actorData.sheet.texture.path = SomeSheet;
    gameData.npcData = {{"rat", rat}, {"spider", spider}};

    PickupData coin;
    coin.sheet.texture.path = SomeSheet;
    coin.size = glm::vec2(16.0f);
    coin.scoreDelta = 1;
    gameData.pickupData = {{"coin", coin}};
    return gameData;
}

inline constexpr int PlatformRow = FloorLevelRow - 3;
inline constexpr glm::ivec2 OnThePlatform{5, PlatformRow - 1};

inline LevelData aFloorWithAPlatform(const std::vector<NpcSpawnData> &npcs)
{
    LevelData levelData = aFloorLevelPlacing(npcs);
    for (int x = 4; x <= 6; ++x)
        levelData.tileMapData.indices[PlatformRow][x] = 1;

    return levelData;
}

inline NpcSpawnData strandedVillager()
{
    NpcSpawnData stranded = spawnAt("rat", OnThePlatform);
    stranded.patrol = beatOf(OnThePlatform, glm::ivec2(5, FloorLevelStanding));
    return stranded;
}

struct Editing
{
    explicit Editing(const std::vector<NpcSpawnData> &extra = {})
        : levelData(aFloorWithAPlatform(extra))
    {
        files.write("floor.json", levelData);
        gameData.levels.first.path = levelPath;
        levels = gameData.levels;
    }

    GameData gameData = aSmallGame();
    TemporaryLevels files{"editor_saving"};
    std::string levelPath = files.pathOf("floor.json");
    LevelData levelData;
    LevelsData levels;
    Level level{
        levelData,
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData};
    TextureCache textures;
    AbilityStates states;
    Observed observed;
    ActorState playerState;
    Camera2D camera{gameData.cameraData, 800, 600};

    EditorSubject subject()
    {
        return EditorSubject{
            gameData,
            level,
            levelData,
            levelPath,
            textures,
            levels,
            states,
            observed,
            level.getTileMap().feetOnTile(glm::ivec2(1, 1)),
            playerState,
            camera,
            false};
    }
};
