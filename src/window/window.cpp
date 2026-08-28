#include <stdexcept>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "window/window.hpp"

Window::Window(int width, int height, const std::string &title)
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!handle)
        throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(handle);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize GLAD");

    glfwSetWindowUserPointer(handle, this);

    glfwSetFramebufferSizeCallback(
        handle,
        [](GLFWwindow *resized, int resizedWidth, int resizedHeight)
        {
            if (Window *window = static_cast<Window *>(glfwGetWindowUserPointer(resized)))
                window->onResize(resizedWidth, resizedHeight);
        });
}

Window::~Window()
{
    if (handle)
    {
        glfwDestroyWindow(handle);
        handle = nullptr;
    }

    glfwTerminate();
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(handle);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(handle);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

void Window::setSize(int width, int height)
{
    glfwSetWindowSize(handle, width, height);
}

glm::ivec2 Window::getFramebufferSize() const
{
    glm::ivec2 size(0);
    glfwGetFramebufferSize(handle, &size.x, &size.y);
    return size;
}

GLFWwindow *Window::getHandle() const
{
    return handle;
}
