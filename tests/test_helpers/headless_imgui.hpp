#pragma once

#include <imgui.h>

class HeadlessImGui
{
public:
    HeadlessImGui()
    {
        context = ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
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
        ImGui::NewFrame();
        ImGui::Begin("headless");
        draw();
        ImGui::End();
        ImGui::Render();
    }

private:
    ImGuiContext *context = nullptr;
};
