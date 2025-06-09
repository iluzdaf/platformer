#pragma once
#include <GLFW/glfw3.h>

class ImGuiManager
{
public:
    ImGuiManager(GLFWwindow *window, const char *glslVersion = "#version 150");
    ~ImGuiManager();
    void newFrame();
    void render();
    ImGuiIO &getIO();

private:
    GLFWwindow *window;
};