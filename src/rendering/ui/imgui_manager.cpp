#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "rendering/ui/imgui_manager.hpp"

ImGuiManager::ImGuiManager(
    GLFWwindow *window,
    int windowWidth,
    int windowHeight,
    const char *glslVersion)
{
    resize(windowWidth, windowHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);
    ImGui::StyleColorsDark();
}

ImGuiManager::~ImGuiManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiManager::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiManager::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

ImGuiIO &ImGuiManager::getIO() const
{
    return ImGui::GetIO();
}

ImVec2 ImGuiManager::worldToScreen(glm::vec2 worldPosition, float zoom, glm::vec2 cameraTopLeft) const
{
    glm::vec2 screenPosition = ((worldPosition - cameraTopLeft) * zoom) / getUiScale();
    return ImVec2(screenPosition.x, screenPosition.y);
}

glm::vec2 ImGuiManager::screenToWorld(ImVec2 screenPosition, float zoom, glm::vec2 cameraTopLeft) const
{
    glm::vec2 screen(screenPosition.x, screenPosition.y);
    glm::vec2 worldPosition = (screen * getUiScale()) / zoom + cameraTopLeft;
    return worldPosition;
}

void ImGuiManager::resize(int width, int height)
{
    if (width <= 0.0f)
        throw std::runtime_error("width must be positive");
    if (height <= 0.0f)
        throw std::runtime_error("height must be positive");

    windowWidth = width;
    windowHeight = height;
}

glm::vec2 ImGuiManager::getUiScale() const
{
    ImVec2 displaySize = getUiDimensions();
    return glm::vec2(windowWidth, windowHeight) / glm::vec2(displaySize.x, displaySize.y);
}

ImVec2 ImGuiManager::getUiDimensions() const
{
    return getIO().DisplaySize;
}

ImDrawList *ImGuiManager::getDrawList()
{
    return ImGui::GetBackgroundDrawList();
}

void ImGuiManager::setNextFullscreenWindow()
{
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(getUiDimensions());
}