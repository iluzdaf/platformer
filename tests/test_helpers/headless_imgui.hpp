#pragma once

#include <imgui.h>
#include <imgui_internal.h>

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

    template <class Draw> void type(const char *into, const char *text, Draw &&draw)
    {
        frame(
            [&]
            {
                ImGui::ActivateItemByID(ImGui::GetID(into));
                draw();
            });
        frame(draw);

        ImGuiIO &io = ImGui::GetIO();
        for (const char *letter = text; *letter != '\0'; ++letter)
            io.AddInputCharacter(static_cast<unsigned int>(*letter));

        frame(draw);
    }

    template <class Draw> void stopTyping(Draw &&draw)
    {
        frame(
            [&]
            {
                ImGui::ClearActiveID();
                draw();
            });
    }

    template <class Draw> void pressEnter(Draw &&draw)
    {
        ImGuiIO &io = ImGui::GetIO();
        io.AddKeyEvent(ImGuiKey_Enter, true);
        frame(draw);
        io.AddKeyEvent(ImGuiKey_Enter, false);
    }

private:
    ImGuiContext *context = nullptr;
};
