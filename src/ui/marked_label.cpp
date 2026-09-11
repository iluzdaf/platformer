#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/unsaved_colours.hpp"

namespace
{
    bool marking = false;
}

inspector::Marking::Marking(bool changed) : before(marking)
{
    marking = changed;
}

inspector::Marking::~Marking()
{
    marking = before;
}

bool inspector::markedHere()
{
    return marking;
}

inspector::Marked::Marked(bool changed) : marked(changed)
{
    if (marked)
        ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
}

inspector::Marked::~Marked()
{
    if (marked)
        ImGui::PopStyleColor();
}

void inspector::drawLabel(std::string_view name, bool changed)
{
    Marked marked(changed);
    ImGui::TextUnformatted(name.data(), name.data() + name.size());
}

void inspector::drawLabel(std::string_view name)
{
    drawLabel(name, markedHere());
}

bool inspector::drawFold(std::string_view name, bool changed)
{
    Marked marked(changed);
    return ImGui::TreeNode(std::string(name).c_str());
}

bool inspector::drawFold(std::string_view name)
{
    return drawFold(name, markedHere());
}
