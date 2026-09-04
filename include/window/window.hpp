#pragma once

#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <signals.hpp>
#include "input/keys_down.hpp"

struct GLFWwindow;

class Window
{
public:
    Window(int width, int height, const std::string &title);
    ~Window();
    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    void setSize(int width, int height);
    glm::ivec2 getFramebufferSize() const;
    GLFWwindow *getHandle() const;
    KeysDown keysDown() const;

    fteng::signal<void(int, int)> onResize;

private:
    GLFWwindow *handle = nullptr;
};
