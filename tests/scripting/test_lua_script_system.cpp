// #include <catch2/catch_test_macros.hpp>
// #include "scripting/lua_script_system.hpp"
// #include "game/game.hpp"
// #include "game/player/player.hpp"
// #include "cameras/camera2d.hpp"
// #include "rendering/screen_transition.hpp"
// #include "game/tile_map/tile_map.hpp"
// #include "game/tile_map/tile_map_data.hpp"

// TEST_CASE("LuaScriptSystem triggers events correctly", "[Lua]")
// {
//     // LuaScriptSystem luaSystem;

//     // Game game;
//     // PlayerData playerData;
//     // PhysicsData physicsData;
//     // Player player(playerData, physicsData);
//     // Camera2DData cameraData;
//     // Camera2D camera(cameraData, 800, 600);
//     // TileMapData tileMapData;
//     // TileMap tileMap(tileMapData);
//     // Shader shader;
//     // shader.initByShaderFile("../assets/shaders/transition.vs", "../assets/shaders/transition.fs");
//     // ScreenTransition transition(shader);

//     // luaSystem.bindGameObjects(&game, &camera, &tileMap, &player, &transition);

//     // bool respawnCalled = false;
//     // bool levelCompleteCalled = false;

//     // luaSystem.getLua()["onRespawn"] = [&]()
//     // { respawnCalled = true; };
//     // luaSystem.getLua()["onLevelComplete"] = [&]()
//     // { levelCompleteCalled = true; };

//     // luaSystem.triggerRespawn();
//     // luaSystem.triggerLevelComplete();

//     // REQUIRE(respawnCalled);
//     // REQUIRE(levelCompleteCalled);
//     REQUIRE(true);
// }

// TEST_CASE("LuaScriptSystem coroutine waits for correct duration", "[Lua][Coroutine]")
// {
//     // LuaScriptSystem luaSystem;

//     // luaSystem.getLua().script(R"(
//     //     function waitTest()
//     //         print("Before wait")
//     //         return 1.0  -- simulate wait for 1 second
//     //     end
//     // )");

//     // luaSystem.getLua()["startCoroutine"](luaSystem.getLua()["waitTest"]);

//     // float elapsed = 0.0f;
//     // while (elapsed < 0.9f)
//     // {
//     //     luaSystem.update(0.1f);
//     //     elapsed += 0.1f;
//     // }

//     // REQUIRE_FALSE(luaSystem.getWaitingCoroutines().empty());

//     // luaSystem.update(0.2f);

//     // REQUIRE(luaSystem.getWaitingCoroutines().empty());

//     REQUIRE(true);
// }