cmake_minimum_required(VERSION 3.31)
project(Platformer)

# The project targets C++23, requested per target with cxx_std_23 further down.
# These are the compilers it is built and tested with; anything else may work but
# is not covered by CI.
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 20)
        message(FATAL_ERROR "clang 20 or newer is required for C++23, found ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 21)
        message(FATAL_ERROR "Apple clang 21 or newer is required for C++23, found ${CMAKE_CXX_COMPILER_VERSION}")
    endif()
elseif(MSVC)
    if(MSVC_VERSION LESS 1938)
        message(FATAL_ERROR "Visual Studio 2022 17.8 or newer is required for C++23, found ${MSVC_VERSION}")
    endif()
else()
    message(FATAL_ERROR "Unsupported compiler ${CMAKE_CXX_COMPILER_ID}; this project targets C++23 with clang, Apple clang or MSVC")
endif()

if(SKIP_OPENGL_TESTS)
    add_compile_definitions(SKIP_OPENGL_TESTS)
endif()

# GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# STB
add_library(stb STATIC external/stb/stb_image.cpp)
target_include_directories(stb PUBLIC external/stb)
target_compile_features(stb PUBLIC cxx_std_23)
set_target_properties(stb PROPERTIES CXX_EXTENSIONS OFF)

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
target_compile_features(imgui PUBLIC cxx_std_23)
set_target_properties(imgui PROPERTIES CXX_EXTENSIONS OFF)
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
    src/rendering/ui/debug_navigation_ui.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/tile_map/tile_interaction_system.cpp
    src/game/player/player.cpp
    src/game/actor/actor.cpp
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
    src/navigation/navigation_graph.cpp
    src/game/npc/behaviors/patrol_behavior.cpp
    src/game/npc/npc.cpp
    src/agent/agent.cpp
    src/agent/movement_abilities/jump_ability.cpp
    src/agent/movement_abilities/dash_ability.cpp
    src/agent/movement_abilities/move_ability.cpp
    src/agent/movement_abilities/wall_slide_ability.cpp
    src/agent/movement_abilities/wall_jump_ability.cpp
    src/agent/movement_abilities/climb_ability.cpp
    src/agent/movement_abilities/climb_move_ability.cpp
    src/agent/movement_abilities/action_buffer.cpp
    src/agent/movement_abilities/coyote_time.cpp
    src/agent/movement_abilities/direction_buffer.cpp
    src/agent/movement_abilities/movement_system.cpp
    src/agent/movement_abilities/gravity_ability.cpp
)

target_compile_features(platformer PRIVATE cxx_std_23)
set_target_properties(platformer PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(platformer PRIVATE
        /Zc:preprocessor
        /external:W0
        /external:anglebrackets
        /W4
        /WX
    )
else()
    target_compile_options(platformer PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-missing-braces
    )
endif()

target_include_directories(platformer
    PRIVATE
    include
    SYSTEM PRIVATE
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
    tests/physics/test_fixed_time_step.cpp
    tests/physics/tests_aabb.cpp
    tests/physics/tests_physics_body.cpp
    tests/cameras/test_camera2d.cpp
    tests/input/test_keyboard_manager.cpp
    tests/animations/test_frame_animation.cpp
    tests/animations/test_sprite_animation.cpp
    tests/animations/test_tile_animation.cpp
    tests/scripting/test_lua_script_system.cpp
    tests/navigation/test_navigation_graph.cpp
    tests/game/npc/behaviors/test_patrol_behavior.cpp
    tests/game/npc/test_npc.cpp
    tests/game/tile_map/test_tile_map_navigation.cpp
    tests/agent/movement_abilities/test_jump_ability.cpp
    tests/agent/movement_abilities/test_dash_ability.cpp
    tests/agent/movement_abilities/test_move_ability.cpp
    tests/agent/movement_abilities/test_wall_slide_ability.cpp
    tests/agent/movement_abilities/test_wall_jump_ability.cpp
    tests/agent/movement_abilities/test_climb_ability.cpp
    tests/agent/movement_abilities/test_climb_move_ability.cpp
    tests/agent/movement_abilities/test_coyote_time.cpp
    tests/agent/movement_abilities/test_action_buffer.cpp
    tests/agent/movement_abilities/test_direction_buffer.cpp
    tests/agent/movement_abilities/test_movement_system.cpp
    tests/agent/movement_abilities/test_gravity_ability.cpp
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
    src/rendering/ui/debug_navigation_ui.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/tile_map/tile_interaction_system.cpp
    src/game/player/player.cpp
    src/game/actor/actor.cpp
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
    src/navigation/navigation_graph.cpp
    src/game/npc/behaviors/patrol_behavior.cpp
    src/game/npc/npc.cpp
    src/agent/agent.cpp
    src/agent/movement_abilities/jump_ability.cpp
    src/agent/movement_abilities/dash_ability.cpp
    src/agent/movement_abilities/move_ability.cpp
    src/agent/movement_abilities/wall_slide_ability.cpp
    src/agent/movement_abilities/wall_jump_ability.cpp
    src/agent/movement_abilities/climb_ability.cpp
    src/agent/movement_abilities/climb_move_ability.cpp
    src/agent/movement_abilities/action_buffer.cpp
    src/agent/movement_abilities/coyote_time.cpp
    src/agent/movement_abilities/direction_buffer.cpp
    src/agent/movement_abilities/movement_system.cpp
    src/agent/movement_abilities/gravity_ability.cpp
    external/stb/stb_image.cpp
)

target_compile_features(tests PRIVATE cxx_std_23)
set_target_properties(tests PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(tests PRIVATE
        /Zc:preprocessor
        /external:W0
        /external:anglebrackets
        /W4
        /WX
    )
else()
    target_compile_options(tests PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-missing-braces
    )
endif()

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
