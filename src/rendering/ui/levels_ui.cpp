#include <cfloat>
#include <string>
#include <imgui.h>
#include "rendering/ui/levels_ui.hpp"
#include "game/levels.hpp"

void LevelsUi::draw(Levels &levels)
{
    saveable.drawControls(
        "levels",
        levels.getFirst(),
        [&levels] { levels.save(); },
        [&levels](const std::string &lastSeen) { levels.setFirst(lastSeen); });

    ImGui::Separator();

    ImGui::TextUnformatted("first");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##first", levelName(levels.getFirst()).c_str()))
    {
        for (const std::string &path : levelPathsIn(directoryOf(levels.getFirst())))
            if (ImGui::Selectable(levelName(path).c_str(), path == levels.getFirst()))
                levels.setFirst(path);

        ImGui::EndCombo();
    }
}

void LevelsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
