#pragma once
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
class Camera2D;
class GLFWwindow;

class ImGuiManager
{
public:
    ImGuiManager(
        GLFWwindow *window,
        int windowWidth,
        int windowHeight,
        const char *glslVersion = "#version 150");
    ~ImGuiManager();
    void newFrame();
    void render();
    ImGuiIO &getIO() const;
    ImVec2 worldToScreen(
        glm::vec2 cameraRelative,
        float zoom,
        glm::vec2 cameraLeftPosition) const;
    glm::vec2 screenToWorld(
        ImVec2 screenPosition,
        float zoom,
        glm::vec2 cameraLeftPosition) const;
    glm::vec2 getUiScale() const;
    void resize(int windowWidth, int windowHeight);
    ImVec2 getUiDimensions() const;

private:
    GLFWwindow *window;
    int windowWidth = 800, windowHeight = 600;
};