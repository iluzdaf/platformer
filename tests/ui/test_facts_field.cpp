#include <cstddef>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "actor/actor_facts.hpp"
#include "actor/actor_fact_rows.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"
#include "helpers/headless_imgui.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/facts_offered_in_scope.hpp"
#include "conditions/asked.hpp"
#include <span>
#include <vector>

namespace
{
    FactsData threeFacts()
    {
        FactsData facts;
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

    Rows rowsOf(HeadlessImGui &gui, FactsData &facts)
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
    STATIC_REQUIRE(inspector::HasCustomField<FactsData>);

    HeadlessImGui gui;
    FactsData facts = threeFacts();

    rowsOf(gui, facts);

    REQUIRE(facts == threeFacts());
}

TEST_CASE("A declared fact can be taken away, and the rest stay", "[FactsField]")
{
    HeadlessImGui gui;
    FactsData facts = threeFacts();

    Rows rows = rowsOf(gui, facts);
    gui.clickAt(
        rows.takeAway(1),
        [&]
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("facts", facts);
        });

    FactsData left = threeFacts();
    left.erase("mood");
    REQUIRE(facts == left);
}

TEST_CASE("Asking to declare opens the chooser and declares nothing by itself", "[FactsField]")
{
    HeadlessImGui gui;
    FactsData facts = threeFacts();

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
    "The facts offered are the rows the engine answers, then what is declared",
    "[FactsField]")
{
    FactsData facts = threeFacts();

    FactsOffered offered = factsOffered(actorRows(), &facts);

    std::span<const FactRow<ActorFacts>> answered = actorRows();
    REQUIRE(offered.size() == answered.size() + 3);
    for (std::size_t at = 0; at < answered.size(); ++at)
    {
        REQUIRE(offered[at].name == answered[at].name);
        REQUIRE(offered[at].kind == answered[at].kind);
    }
    std::size_t declared = answered.size();
    REQUIRE(offered[declared].name == "hits");
    REQUIRE(offered[declared].kind == AskedKind::Number);
    REQUIRE(offered[declared + 1].name == "mood");
    REQUIRE(offered[declared + 1].kind == AskedKind::Name);
    REQUIRE(offered[declared + 2].name == "near");
    REQUIRE(offered[declared + 2].kind == AskedKind::YesOrNo);

    REQUIRE(factsOffered(actorRows()).size() == answered.size());
}
