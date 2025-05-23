cmake_minimum_required(VERSION 3.10)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)

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

# Platformer Executable
add_executable(platformer
    src/main.cpp
    src/rendering/texture2d.cpp
    src/rendering/sprite_renderer.cpp
    src/rendering/shader.cpp
    src/rendering/tile_map_renderer.cpp
    src/game/tile_map.cpp
    src/game/player.cpp
    src/game/fixed_time_step.cpp
    src/game/frame_animation.cpp
    src/game/sprite_animation.cpp
)

target_include_directories(platformer
    PRIVATE
    include
    external/glad/include
    external/glfw/include
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
    tests/test_texture2d.cpp
    tests/test_shader.cpp
    tests/test_tile_map.cpp
    tests/test_player.cpp
    tests/test_fixed_time_step.cpp
    tests/test_frame_animation.cpp
    tests/test_sprite_animation.cpp
    tests/test_helpers/gl_context.cpp
    src/rendering/texture2d.cpp
    src/rendering/shader.cpp
    src/game/tile_map.cpp
    src/game/player.cpp
    src/game/fixed_time_step.cpp
    src/game/frame_animation.cpp
    src/game/sprite_animation.cpp
    external/stb/stb_image.cpp
)

target_include_directories(tests
    PRIVATE
    include
    external/glad/include
    external/glfw/include
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
