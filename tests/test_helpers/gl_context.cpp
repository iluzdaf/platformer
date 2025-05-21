#include "gl_context.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

void initOpenGLForTests()
{
    static bool initialized = false;
    if (initialized)
        return;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        std::exit(1);
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1, 1, "", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create OpenGL test context\n";
        glfwTerminate();
        std::exit(1);
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        std::exit(1);
    }

    initialized = true;
}