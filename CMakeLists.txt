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

# Catch2 (for tests only)
add_subdirectory(external/catch2)

# Glaze
add_subdirectory(external/glaze)

# Platformer Executable
add_executable(platformer
    src/main.cpp
    src/rendering/texture2d.cpp
    src/rendering/sprite_renderer.cpp
    src/rendering/shader.cpp
    src/rendering/tile_map_renderer.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/player/player.cpp
    src/physics/fixed_time_step.cpp
    src/cameras/camera2d.cpp
    src/input/keyboard_manager.cpp
    src/animations/frame_animation.cpp
    src/animations/sprite_animation.cpp
    src/animations/tile_animation.cpp
)

target_include_directories(platformer
    PRIVATE
    include
    external/glad/include
    external/glfw/include
    external/glaze/include
)

target_link_libraries(platformer
    PRIVATE
    glad
    glfw
    stb
    ${CMAKE_DL_LIBS}
)

# Tests
add_executable(tests
    tests/test_main.cpp
    tests/rendering/test_texture2d.cpp
    tests/rendering/test_shader.cpp
    src/rendering/texture2d.cpp
    src/rendering/shader.cpp
    tests/game/tile_map/test_tile_map.cpp
    tests/game/tile_map/test_tile.cpp
    tests/game/player/test_player.cpp
    tests/physics/test_fixed_time_step.cpp
    tests/cameras/test_camera2d.cpp
    tests/input/test_keyboard_manager.cpp
    tests/animations/test_frame_animation.cpp
    tests/animations/test_sprite_animation.cpp
    tests/animations/test_tile_animation.cpp
    tests/test_helpers/gl_context.cpp
    src/game/tile_map/tile_map.cpp
    src/game/tile_map/tile.cpp
    src/game/player/player.cpp
    src/physics/fixed_time_step.cpp
    src/cameras/camera2d.cpp
    src/input/keyboard_manager.cpp
    src/animations/frame_animation.cpp
    src/animations/sprite_animation.cpp
    src/animations/tile_animation.cpp
    external/stb/stb_image.cpp
)

target_include_directories(tests
    PRIVATE
    include
    external/glad/include
    external/glfw/include
    external/glaze/include
)

target_link_libraries(tests
    PRIVATE
    Catch2::Catch2WithMain
    glad
    glfw
    stb
    ${CMAKE_DL_LIBS}
)

enable_testing()
add_test(NAME AllTests COMMAND tests)
