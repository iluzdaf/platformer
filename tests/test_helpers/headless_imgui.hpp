#pragma once

#include <imgui.h>

class HeadlessImGui
{
public:
    HeadlessImGui()
    {
        context = ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.DisplaySize = ImVec2(800.0f, 600.0f);
        io.DeltaTime = 1.0f / 60.0f;

        unsigned char *pixels = nullptr;
        int width = 0, height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    HeadlessImGui(const HeadlessImGui &) = delete;
    HeadlessImGui &operator=(const HeadlessImGui &) = delete;

    ~HeadlessImGui()
    {
        ImGui::DestroyContext(context);
    }

    template <class Draw> void frame(Draw &&draw)
    {
        frameWithMouse(ImVec2(-1.0f, -1.0f), false, draw);
    }

    template <class Draw> void frameWithMouse(ImVec2 mouse, bool held, Draw &&draw)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.MousePos = mouse;
        io.MouseDown[0] = held;

        ImGui::NewFrame();
        ImGui::Begin("headless");
        draw();
        ImGui::End();
        ImGui::Render();
    }

    template <class Draw> void clickAt(ImVec2 at, Draw &&draw)
    {
        frameWithMouse(at, false, draw);
        frameWithMouse(at, true, draw);
        frameWithMouse(at, true, draw);
        frameWithMouse(at, false, draw);
    }

private:
    ImGuiContext *context = nullptr;
};
