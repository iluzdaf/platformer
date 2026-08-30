#include <cfloat>
#include <string>
#include <imgui.h>
#include "rendering/ui/levels_ui.hpp"
#include "game/levels.hpp"
#include "game/level.hpp"
#include "rendering/ui/editor_commands.hpp"

void LevelsUi::draw(Levels &levels, const Level &level, EditorCommands &commands)
{
    ImGui::TextUnformatted("playing");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##playing", levelName(level.getPath()).c_str()))
    {
        for (const std::string &path : levelPathsIn(directoryOf(level.getPath())))
        {
            bool current = path == level.getPath();
            if (ImGui::Selectable(levelName(path).c_str(), current) && !current)
                commands.onLoadLevel(path);
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

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
