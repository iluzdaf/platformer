#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
class Camera2D;

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
    ImGuiIO &getIO();
    ImVec2 cameraRelativeToScreen(
        glm::vec2 cameraRelative,
        float zoom) const;
    glm::vec2 screenToWorld(
        ImVec2 screenPosition,
        const Camera2D &camera) const;
    void update();
    glm::vec2 getUiScale() const;
    void resize(int windowWidth, int windowHeight);
    ImVec2 getUiDimensions() const;

private:
    GLFWwindow *window;
    int windowWidth = 800, windowHeight = 600,
        uiWidth = 800, uiHeight = 600;
    glm::vec2 uiScale = glm::vec2(1.0f);
};