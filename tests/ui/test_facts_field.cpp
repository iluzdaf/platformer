#include <cstddef>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "conditions/facts.hpp"
#include "helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/when_field.hpp"
#include "conditions/asked.hpp"
#include <vector>

namespace
{
    Facts threeFacts()
    {
        Facts facts;
        facts["hits"] = 2.0f;
        facts["mood"] = std::string("calm");
        facts["near"] = true;
        return facts;
    }

    ImVec2 centreOf(ImVec2 low, ImVec2 high)
    {
        return ImVec2((low.x + high.x) * 0.5f, (low.y + high.y) * 0.5f);
    }

    struct Rows
    {
        ImVec2 add;
        float top = 0.0f, x = 0.0f;

        ImVec2 takeAway(std::size_t row) const
        {
            return ImVec2(
                x,
                top + ImGui::GetTextLineHeightWithSpacing() +
                    static_cast<float>(row) * ImGui::GetFrameHeightWithSpacing() +
                    ImGui::GetFrameHeight() * 0.5f);
        }
    };

    Rows rowsOf(HeadlessImGui &gui, Facts &facts)
    {
        Rows found;
        auto drawOpen = [&]
        {
            found.top = ImGui::GetCursorScreenPos().y;
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("facts", facts);
            found.add = centreOf(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            found.x = found.add.x;
        };

        gui.frame(drawOpen);
        gui.frame(drawOpen);

        return found;
    }
}

TEST_CASE("Declared facts draw themselves, and keep what they were given", "[FactsField]")
{
    STATIC_REQUIRE(inspector::HasCustomField<Facts>);

    HeadlessImGui gui;
    Facts facts = threeFacts();

    rowsOf(gui, facts);

    REQUIRE(facts == threeFacts());
}

TEST_CASE("A declared fact can be taken away, and the rest stay", "[FactsField]")
{
    HeadlessImGui gui;
    Facts facts = threeFacts();

    Rows rows = rowsOf(gui, facts);
    gui.clickAt(
        rows.takeAway(1),
        [&]
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("facts", facts);
        });

    Facts left = threeFacts();
    left.erase("mood");
    REQUIRE(facts == left);
}

TEST_CASE("Asking to declare opens the chooser and declares nothing by itself", "[FactsField]")
{
    HeadlessImGui gui;
    Facts facts = threeFacts();

    Rows rows = rowsOf(gui, facts);
    gui.clickAt(
        rows.add,
        [&]
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("facts", facts);
        });

    REQUIRE(facts == threeFacts());
}

TEST_CASE(
    "A condition is offered the ground the engine answers, then what is declared",
    "[FactsField]")
{
    Facts facts = threeFacts();

    std::vector<FactOffered> offered = factsOffered(&facts);

    REQUIRE(offered.size() == 4);
    REQUIRE(offered[0].name == "onGround");
    REQUIRE(offered[0].kind == AskedKind::YesOrNo);
    REQUIRE(offered[1].name == "hits");
    REQUIRE(offered[1].kind == AskedKind::Number);
    REQUIRE(offered[2].name == "mood");
    REQUIRE(offered[2].kind == AskedKind::Name);
    REQUIRE(offered[3].name == "near");
    REQUIRE(offered[3].kind == AskedKind::YesOrNo);

    REQUIRE(factsOffered(nullptr).size() == 1);
}
