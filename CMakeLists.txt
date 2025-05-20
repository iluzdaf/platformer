cmake_minimum_required(VERSION 3.10)
project(Platformer)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(glfw)
include_directories(include glfw/include)

add_executable(platformer src/main.cpp src/glad.c)
target_link_libraries(platformer glfw ${CMAKE_DL_LIBS})