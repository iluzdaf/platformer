#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include "rendering/ui/imgui_manager.hpp"
#include "cameras/camera2d.hpp"

ImGuiManager::ImGuiManager(
    GLFWwindow *window,
    int windowWidth,
    int windowHeight,
    const char *glslVersion)
    : window(window)
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
    glm::vec2 screenPosition = ((worldPosition - cameraTopLeft) * zoom) / uiScale;
    return ImVec2(screenPosition.x, screenPosition.y);
}

glm::vec2 ImGuiManager::screenToWorld(ImVec2 screenPosition, float zoom, glm::vec2 cameraTopLeft) const
{
    glm::vec2 screen(screenPosition.x, screenPosition.y);
    glm::vec2 worldPosition = (screen * uiScale) / zoom + cameraTopLeft;
    return worldPosition;
}

void ImGuiManager::update()
{
    ImGuiIO &io = getIO();
    uiWidth = io.DisplaySize.x;
    uiHeight = io.DisplaySize.y;
    uiScale = glm::vec2(windowWidth, windowHeight) / glm::vec2(uiWidth, uiHeight);
}

void ImGuiManager::resize(int windowWidth, int windowHeight)
{
    if (windowWidth <= 0.0f)
        throw std::invalid_argument("windowWidth must be positive");
    if (windowHeight <= 0.0f)
        throw std::invalid_argument("windowHeight must be positive");

    this->windowWidth = windowWidth;
    this->windowHeight = windowHeight;
}

glm::vec2 ImGuiManager::getUiScale() const
{
    return uiScale;
}

ImVec2 ImGuiManager::getUiDimensions() const
{
    return ImVec2(uiWidth, uiHeight);
}