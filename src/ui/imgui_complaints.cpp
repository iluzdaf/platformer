#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <imgui.h>
#include <imgui_internal.h>
#include "ui/imgui_complaints.hpp"

namespace
{
    struct Watching
    {
        std::unordered_map<ImGuiID, int> timesDrawn;
        std::unordered_map<ImGuiID, std::string> labels;
        std::vector<std::string> said;
        std::unordered_set<std::string> saidBefore;
        ImGuiID clashing = 0;
    };

    Watching *watching = nullptr;

    void complain(std::string what)
    {
        if (!watching->saidBefore.insert(what).second)
            return;

        std::cerr << "imgui: " << what << '\n';
        watching->said.push_back(std::move(what));
    }

    void imGuiSaid(ImGuiContext *context, void *, const char *message)
    {
        if (!watching)
            return;

        const ImGuiWindow *window = context == nullptr ? nullptr : context->CurrentWindow;
        complain(std::string("in window \"") + (window ? window->Name : "none") + "\": " + message);
    }
}

void ImGuiTestEngineHook_ItemAdd(
    ImGuiContext *,
    ImGuiID id,
    const ImRect &,
    const ImGuiLastItemData *itemData)
{
    if (!watching || (itemData && (itemData->ItemFlags & ImGuiItemFlags_AllowDuplicateId) != 0))
        return;

    if (++watching->timesDrawn[id] != 2)
        return;

    auto known = watching->labels.find(id);
    if (known != watching->labels.end())
    {
        complain("two items were drawn with the same id, called \"" + known->second + "\"");
        return;
    }

    watching->clashing = id;
    complain("two items were drawn with the same id");
}

void ImGuiTestEngineHook_ItemInfo(
    ImGuiContext *,
    ImGuiID id,
    const char *label,
    ImGuiItemStatusFlags)
{
    if (!watching || label == nullptr)
        return;

    watching->labels[id] = label;
    if (watching->clashing != id)
        return;

    watching->clashing = 0;
    watching->said.back() += ", called \"" + std::string(label) + "\"";
}

void ImGuiTestEngineHook_Log(ImGuiContext *, const char *, ...)
{
}

const char *ImGuiTestEngine_FindItemDebugLabel(ImGuiContext *, ImGuiID)
{
    return nullptr;
}

namespace
{
    Watching heard;
}

ImGuiComplaints::ImGuiComplaints()
{
    startAgain();
    watching = &heard;

    ImGuiContext *context = ImGui::GetCurrentContext();
    context->TestEngineHookItems = true;
    context->ErrorCallback = imGuiSaid;
    context->IO.ConfigErrorRecoveryEnableAssert = false;
    context->DebugLogFlags &= ~ImGuiDebugLogFlags_OutputToTTY;
}

ImGuiComplaints::~ImGuiComplaints()
{
    watching = nullptr;

    if (ImGuiContext *context = ImGui::GetCurrentContext())
    {
        context->TestEngineHookItems = false;
        context->ErrorCallback = nullptr;
    }
}

void ImGuiComplaints::startAgain()
{
    heard.timesDrawn.clear();
    heard.said.clear();
    heard.clashing = 0;
}

std::optional<std::string> ImGuiComplaints::anything() const
{
    if (heard.said.empty())
        return std::nullopt;

    std::string all = heard.said.front();
    for (std::size_t at = 1; at < heard.said.size(); ++at)
        all += "; " + heard.said[at];

    return all;
}
