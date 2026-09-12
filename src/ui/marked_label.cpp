#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/marked_label.hpp"
#include "ui/section_mark.hpp"
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

inspector::Marked::Marked(bool changed) : Marked(changed, false)
{
}

inspector::Marked::Marked(bool changed, bool refused) : marked(markFor(changed, refused))
{
    if (marked)
        ImGui::PushStyleColor(ImGuiCol_Text, *marked);
}

inspector::Marked::~Marked()
{
    if (marked)
        ImGui::PopStyleColor();
}

void inspector::drawRefusal(std::string_view why)
{
    ImGui::PushStyleColor(ImGuiCol_Text, CannotSaveColour);
    ImGui::TextWrapped("%s", std::string(why).c_str());
    ImGui::PopStyleColor();
}

void inspector::drawLabel(std::string_view name, bool changed, bool refused)
{
    Marked marked(changed, refused);
    ImGui::TextUnformatted(name.data(), name.data() + name.size());
}

void inspector::drawLabel(std::string_view name, bool changed)
{
    drawLabel(name, changed, false);
}

void inspector::drawLabel(std::string_view name)
{
    drawLabel(name, markedHere(), false);
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
