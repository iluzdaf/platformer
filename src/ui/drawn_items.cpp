#include <cstdarg>
#include <map>
#include <optional>
#include <string>
#include <imgui.h>
#include <imgui_internal.h>
#include "ui/drawn_items.hpp"

namespace
{
    struct Drawn
    {
        int times = 0;
        std::string label;
    };

    std::map<ImGuiID, Drawn> *watching = nullptr;
    std::map<ImGuiID, Drawn> drawn;
}

void ImGuiTestEngineHook_ItemAdd(
    ImGuiContext *,
    ImGuiID id,
    const ImRect &,
    const ImGuiLastItemData *itemData)
{
    if (!watching || (itemData && (itemData->ItemFlags & ImGuiItemFlags_AllowDuplicateId) != 0))
        return;

    (*watching)[id].times++;
}

void ImGuiTestEngineHook_ItemInfo(
    ImGuiContext *,
    ImGuiID id,
    const char *label,
    ImGuiItemStatusFlags)
{
    if (!watching || label == nullptr)
        return;

    (*watching)[id].label = label;
}

void ImGuiTestEngineHook_Log(ImGuiContext *, const char *, ...)
{
}

const char *ImGuiTestEngine_FindItemDebugLabel(ImGuiContext *, ImGuiID id)
{
    if (!watching)
        return nullptr;

    auto found = watching->find(id);
    return found == watching->end() || found->second.label.empty() ? nullptr
                                                                   : found->second.label.c_str();
}

DrawnItems::DrawnItems()
{
    drawn.clear();
    watching = &drawn;
    ImGui::GetCurrentContext()->TestEngineHookItems = true;
}

DrawnItems::~DrawnItems()
{
    watching = nullptr;
    if (ImGui::GetCurrentContext())
        ImGui::GetCurrentContext()->TestEngineHookItems = false;
}

void DrawnItems::startAgain()
{
    drawn.clear();
}

std::optional<std::string> DrawnItems::twoWithTheSameId() const
{
    for (const auto &[id, item] : drawn)
        if (item.times > 1)
            return "two items share an id, one of them called \"" + item.label + "\"";

    return std::nullopt;
}
