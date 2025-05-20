cmake_minimum_required(VERSION 3.10)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(glfw)
include_directories(include glfw/include)

add_subdirectory(catch2)

add_executable(tests
    tests/test_stub.cpp
)

target_include_directories(tests PRIVATE include)
target_link_libraries(tests Catch2::Catch2WithMain)

enable_testing()
add_test(NAME AllTests COMMAND tests)

add_executable(platformer src/main.cpp src/glad.c)
target_link_libraries(platformer glfw ${CMAKE_DL_LIBS})