cmake_minimum_required(VERSION 3.31)
project(Platformer)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 20)
        message(FATAL_ERROR "clang 20 or newer is required for C++23, found ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(MSVC)
    if(MSVC_VERSION LESS 1938)
        message(FATAL_ERROR "Visual Studio 2022 17.8 or newer is required for C++23, found ${MSVC_VERSION}")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}; this project targets C++23 with LLVM clang or MSVC. Apple clang trails on C++23 and is not tested; on macOS use brew install llvm")
endif()

set(CMAKE_CXX_SCAN_FOR_MODULES OFF CACHE BOOL "Scan sources for C++20 module dependencies")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(intelliSenseBuildDirectories Debug Release RelWithDebInfo MinSizeRel)
get_filename_component(buildDirectoryName ${CMAKE_BINARY_DIR} NAME)
get_filename_component(buildDirectoryParent ${CMAKE_BINARY_DIR} DIRECTORY)

if(NOT MSVC
    AND buildDirectoryParent STREQUAL ${CMAKE_SOURCE_DIR}/build
    AND buildDirectoryName IN_LIST intelliSenseBuildDirectories)
    file(CREATE_LINK
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
        SYMBOLIC)
endif()

if(SKIP_OPENGL_TESTS)
    add_compile_definitions(SKIP_OPENGL_TESTS)
endif()

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

add_library(stb STATIC external/stb/stb_image.cpp)
target_include_directories(stb PUBLIC external/stb)
target_compile_features(stb PUBLIC cxx_std_23)
set_target_properties(stb PROPERTIES CXX_EXTENSIONS OFF)

add_subdirectory(external/glfw)

include_directories(external/glm)

include_directories(external/signals)

set(CATCH_CONFIG_CPP17_STRING_VIEW ON CACHE BOOL "" FORCE)
add_subdirectory(external/catch2)
target_compile_features(Catch2 PUBLIC cxx_std_17)

add_subdirectory(external/glaze)

file(GLOB LUA_SOURCES
    external/lua/lua-5.4.7/src/*.c
)
add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC external/lua/lua-5.4.7/include)

add_subdirectory(external/sol2)
include_directories(external/sol2/include)

file(GLOB IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)
target_compile_features(imgui PUBLIC cxx_std_23)
set_target_properties(imgui PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(imgui PRIVATE glfw glad)

add_subdirectory(external/efsw)

add_library(platformer_lib OBJECT
    src/rendering/texture2d.cpp
    src/rendering/sprite_renderer.cpp
    src/rendering/shader.cpp
    src/rendering/game_renderer.cpp
    src/rendering/tile_map_drawing.cpp
    src/rendering/screen_transition.cpp
    src/rendering/ui/editor_ui.cpp
    src/rendering/ui/game_editor_ui.cpp
    src/rendering/ui/game_ui.cpp
    src/rendering/ui/imgui_manager.cpp
    src/rendering/ui/tile_map_overlays.cpp
    src/rendering/ui/debug_aabb_ui.cpp
    src/rendering/ui/level_editor_ui.cpp
    src/rendering/ui/score_ui.cpp
    src/rendering/ui/navigation_overlay.cpp
    src/rendering/ui/navigation_ui.cpp
    src/rendering/ui/npcs_ui.cpp
    src/tile_map/tile_map.cpp
    src/tile_map/tile.cpp
    src/tile_map/tile_interaction_system.cpp
    src/player/player.cpp
    src/actor/actor.cpp
    src/assets/asset_paths.cpp
    src/reloading/hot_reload.cpp
    src/reloading/reloader.cpp
    src/app/app.cpp
    src/window/window.cpp
    src/game/game.cpp
    src/game/game_data.cpp
    src/game/playback.cpp
    src/serialization/json_format.cpp
    src/game/level.cpp
    src/game/levels.cpp
    src/game/scoring_system.cpp
    src/physics/aabb.cpp
    src/timing/fixed_time_step.cpp
    src/physics/physics_body.cpp
    src/cameras/camera2d.cpp
    src/cameras/camera_shake.cpp
    src/input/keyboard_manager.cpp
    src/input/input_manager.cpp
    src/animations/frame_animation.cpp
    src/animations/sprite_animation.cpp
    src/animations/tile_animation.cpp
    src/animations/animation_manager.cpp
    src/scripting/lua_script_system.cpp
    src/navigation/navigation_graph.cpp
    src/navigation/navigation_graph_builder.cpp
    src/navigation/navigation_path.cpp
    src/navigation/jump_simulation.cpp
    src/navigation/navigation_profile_builder.cpp
    src/actor/actor_behavior.cpp
    src/actor/behaviors/input_behavior.cpp
    src/actor/behaviors/flee_behavior.cpp
    src/actor/behaviors/patrol_behavior.cpp
    src/actor/behaviors/route_walker.cpp
    src/actor/behaviors/state_machine_behavior.cpp
    src/npc/npc.cpp
    src/actor/actor_motion.cpp
    src/actor/abilities/jump_ability.cpp
    src/actor/abilities/dash_ability.cpp
    src/actor/abilities/move_ability.cpp
    src/actor/abilities/wall_slide_ability.cpp
    src/actor/abilities/wall_jump_ability.cpp
    src/actor/abilities/climb_ability.cpp
    src/actor/abilities/climb_move_ability.cpp
    src/actor/abilities/action_buffer.cpp
    src/actor/abilities/coyote_time.cpp
    src/actor/abilities/direction_buffer.cpp
    src/actor/abilities/ability_system.cpp
    src/actor/abilities/gravity_ability.cpp
)

add_executable(platformer src/main.cpp)

target_compile_features(platformer_lib PUBLIC cxx_std_23)
set_target_properties(platformer_lib PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(platformer_lib PUBLIC
        /Zc:preprocessor
        /external:W0
        /external:anglebrackets
        /W4
        /WX
    )
else()
    target_compile_options(platformer_lib PUBLIC
        -Wall
        -Wextra
        -Werror
        -Wno-missing-braces
    )
endif()

target_compile_definitions(platformer_lib PUBLIC PLATFORMER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

target_include_directories(platformer_lib PUBLIC include)

target_include_directories(platformer_lib SYSTEM PUBLIC
    external/glad/include
    external/glfw/include
    external/glaze/include
    external/sol2/include
    external/efsw/include
)

target_link_libraries(platformer_lib
    PUBLIC
    glad
    glfw
    stb
    lua
    imgui
    efsw
    ${CMAKE_DL_LIBS}
)

target_link_libraries(platformer PRIVATE platformer_lib)

set(selfContainmentSources)

foreach(headerRoot include tests)
    file(GLOB_RECURSE selfContainmentHeaders CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/${headerRoot}/*.hpp)

    foreach(header ${selfContainmentHeaders})
        file(RELATIVE_PATH includedPath ${CMAKE_SOURCE_DIR}/${headerRoot} ${header})
        string(REPLACE "/" "_" translationUnitName ${includedPath})
        set(translationUnit ${CMAKE_BINARY_DIR}/header_self_containment/${translationUnitName}.cpp)
        file(WRITE ${translationUnit} "#include \"${includedPath}\"\n")
        list(APPEND selfContainmentSources ${translationUnit})
    endforeach()
endforeach()

add_library(header_self_containment OBJECT EXCLUDE_FROM_ALL ${selfContainmentSources})

target_compile_definitions(header_self_containment PRIVATE PLATFORMER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

target_include_directories(header_self_containment PRIVATE tests)

target_link_libraries(header_self_containment PRIVATE platformer_lib)

add_executable(tests
    tests/test_main.cpp
    tests/rendering/test_texture2d.cpp
    tests/rendering/test_gl_ownership.cpp
    tests/rendering/test_shader.cpp
    tests/tile_map/test_tile_map.cpp
    tests/tile_map/test_tile.cpp
    tests/tile_map/test_tile_interaction_system.cpp
    tests/player/test_player.cpp
    tests/timing/test_fixed_time_step.cpp
    tests/physics/tests_aabb.cpp
    tests/physics/tests_physics_body.cpp
    tests/cameras/test_camera2d.cpp
    tests/cameras/test_camera_shake.cpp
    tests/input/test_keyboard_manager.cpp
    tests/animations/test_animation_manager.cpp
    tests/animations/test_frame_animation.cpp
    tests/animations/test_sprite_animation.cpp
    tests/animations/test_tile_animation.cpp
    tests/navigation/test_navigation_graph.cpp
    tests/scripting/test_lua_script_system.cpp
    tests/navigation/test_navigation_graph_builder.cpp
    tests/navigation/test_navigation_path.cpp
    tests/navigation/test_jump_simulation.cpp
    tests/actor/behaviors/test_flee_behavior.cpp
    tests/actor/behaviors/test_patrol_behavior.cpp
    tests/actor/behaviors/test_state_machine_behavior.cpp
    tests/npc/test_npc.cpp
    tests/assets/test_asset_paths.cpp
    tests/reloading/test_reloader.cpp
    tests/game/test_playback.cpp
    tests/game/test_level.cpp
    tests/game/test_levels.cpp
    tests/tile_map/test_tile_map_navigation.cpp
    tests/actor/abilities/test_jump_ability.cpp
    tests/actor/abilities/test_dash_ability.cpp
    tests/actor/abilities/test_move_ability.cpp
    tests/actor/abilities/test_wall_slide_ability.cpp
    tests/actor/abilities/test_wall_jump_ability.cpp
    tests/actor/abilities/test_climb_ability.cpp
    tests/actor/abilities/test_climb_move_ability.cpp
    tests/actor/abilities/test_coyote_time.cpp
    tests/actor/abilities/test_action_buffer.cpp
    tests/actor/abilities/test_direction_buffer.cpp
    tests/actor/abilities/test_ability_system.cpp
    tests/actor/abilities/test_gravity_ability.cpp
    external/stb/stb_image.cpp
)

target_compile_definitions(tests PRIVATE PLATFORMER_ASSETS_DIR="${CMAKE_SOURCE_DIR}/assets")

target_include_directories(tests PRIVATE tests/)

target_link_libraries(tests
    PRIVATE
    platformer_lib
    Catch2::Catch2WithMain
)

enable_testing()
add_test(NAME AllTests COMMAND tests)
