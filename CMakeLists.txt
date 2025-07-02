cmake_minimum_required(VERSION 3.31)
project(Platformer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/Zc:preprocessor)
endif()

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# STB
add_library(stb STATIC external/stb/stb_image.cpp)
target_include_directories(stb PUBLIC external/stb)

# GLFW
add_subdirectory(external/glfw)

# GLM (Header-only)
include_directories(external/glm)

# TheWisp/signals (Header-only)
include_directories(external/signals)

# Catch2 (for tests only)
add_subdirectory(external/catch2)

# Glaze
add_subdirectory(external/glaze)

# Lua
# add_subdirectory(external/lua)
file(GLOB LUA_SOURCES
    external/lua/lua-5.4.7/src/*.c
)
add_library(lua STATIC ${LUA_SOURCES})
target_include_directories(lua PUBLIC external/lua/lua-5.4.7/include)

# sol2 (header-only)
add_subdirectory(external/sol2)
include_directories(external/sol2/include)

# IMGUI
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
target_link_libraries(imgui PRIVATE glfw glad)

#esfw
add_subdirectory(external/efsw)

# Platformer Executable
add_executable(platformer
    src/main.cpp
    src/rendering/texture2d.cpp
    src/rendering/sprite_renderer.cpp
    src/rendering/shader.cpp
    src/rendering/tile_map_renderer.cpp
    src/rendering/screen_transition.cpp
    src/rendering/ui/debug_ui.cpp
    src/rendering/ui/imgui_manager.cpp
    src/rendering/ui/debug_tile_map_ui.cpp
    src/rendering/ui/debug_aabb_ui.cpp
    src/rendering/ui/editor_tile_map_ui.cpp
    src/rendering/ui/score_ui.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/tile_map/tile_interaction_system.cpp
    src/game/player/player.cpp
    src/game/player/movement_abilities/jump_ability.cpp
    src/game/player/movement_abilities/dash_ability.cpp
    src/game/player/movement_abilities/move_ability.cpp
    src/game/player/movement_abilities/wall_slide_ability.cpp
    src/game/player/movement_abilities/wall_jump_ability.cpp
    src/game/player/movement_abilities/climb_ability.cpp
    src/game/player/movement_abilities/climb_move_ability.cpp
    src/game/player/movement_abilities/movement_system.cpp
    src/game/game.cpp
    src/game/scoring_system.cpp
    src/physics/fixed_time_step.cpp
    src/physics/physics_body.cpp
    src/cameras/camera2d.cpp
    src/input/keyboard_manager.cpp
    src/input/input_manager.cpp
    src/animations/frame_animation.cpp
    src/animations/sprite_animation.cpp
    src/animations/tile_animation.cpp
    src/animations/animation_manager.cpp
    src/scripting/lua_script_system.cpp
)

target_include_directories(platformer
    PRIVATE
    include
    external/glad/include
    external/glfw/include
    external/glaze/include
    external/sol2/include
    external/efsw/include
)

target_link_libraries(platformer
    PRIVATE
    glad
    glfw
    stb
    lua
    imgui
    efsw
    ${CMAKE_DL_LIBS}
)

# Tests
add_executable(tests
    tests/test_main.cpp
    tests/rendering/test_texture2d.cpp
    tests/rendering/test_shader.cpp
    tests/game/tile_map/test_tile_map.cpp
    tests/game/tile_map/test_tile.cpp
    tests/game/tile_map/test_tile_interaction_system.cpp
    tests/game/player/test_player.cpp
    tests/game/player/movement_abilities/test_jump_ability.cpp
    tests/game/player/movement_abilities/test_dash_ability.cpp
    tests/game/player/movement_abilities/test_move_ability.cpp
    tests/game/player/movement_abilities/test_wall_slide_ability.cpp
    tests/game/player/movement_abilities/test_wall_jump_ability.cpp
    tests/game/player/movement_abilities/test_climb_ability.cpp
    tests/game/player/movement_abilities/test_climb_move_ability.cpp
    tests/game/player/movement_abilities/test_coyote_time.cpp
    tests/game/player/movement_abilities/test_action_buffer.cpp
    tests/game/player/movement_abilities/test_direction_buffer.cpp
    tests/game/player/movement_abilities/test_movement_system.cpp
    tests/physics/test_fixed_time_step.cpp
    tests/physics/tests_aabb.cpp
    tests/physics/tests_physics_body.cpp
    tests/cameras/test_camera2d.cpp
    tests/input/test_keyboard_manager.cpp
    tests/animations/test_frame_animation.cpp
    tests/animations/test_sprite_animation.cpp
    tests/animations/test_tile_animation.cpp
    tests/scripting/test_lua_script_system.cpp
    src/rendering/texture2d.cpp
    src/rendering/shader.cpp
    src/rendering/screen_transition.cpp
    src/rendering/sprite_renderer.cpp
    src/rendering/tile_map_renderer.cpp
    src/rendering/ui/debug_ui.cpp
    src/rendering/ui/imgui_manager.cpp
    src/rendering/ui/debug_tile_map_ui.cpp
    src/rendering/ui/debug_aabb_ui.cpp
    src/rendering/ui/editor_tile_map_ui.cpp
    src/rendering/ui/score_ui.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/tile_map/tile_interaction_system.cpp
    src/game/player/player.cpp
    src/game/player/movement_abilities/jump_ability.cpp
    src/game/player/movement_abilities/dash_ability.cpp
    src/game/player/movement_abilities/move_ability.cpp
    src/game/player/movement_abilities/wall_slide_ability.cpp
    src/game/player/movement_abilities/wall_jump_ability.cpp
    src/game/player/movement_abilities/climb_ability.cpp
    src/game/player/movement_abilities/climb_move_ability.cpp
    src/game/player/movement_abilities/movement_system.cpp
    src/game/game.cpp
    src/game/scoring_system.cpp
    src/physics/fixed_time_step.cpp
    src/physics/physics_body.cpp
    src/cameras/camera2d.cpp
    src/input/keyboard_manager.cpp
    src/input/input_manager.cpp
    src/animations/frame_animation.cpp
    src/animations/sprite_animation.cpp
    src/animations/tile_animation.cpp
    src/animations/animation_manager.cpp
    src/scripting/lua_script_system.cpp
    external/stb/stb_image.cpp
)

target_include_directories(tests
    PRIVATE
    include
    external/glad/include
    external/glfw/include
    external/glaze/include
    external/sol2/include
    external/efsw/include
    tests/
)

target_link_libraries(tests
    PRIVATE
    Catch2::Catch2WithMain
    glad
    glfw
    stb
    lua
    imgui
    efsw
    ${CMAKE_DL_LIBS}
)

enable_testing()
add_test(NAME AllTests COMMAND tests)
