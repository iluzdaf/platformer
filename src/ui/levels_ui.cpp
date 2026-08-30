#include <cfloat>
#include <optional>
#include <string>
#include <imgui.h>
#include "ui/levels_ui.hpp"
#include "game/levels.hpp"
#include "game/level.hpp"
#include "ui/editor_commands.hpp"

namespace
{
    std::optional<std::string> levelChooser(const char *label, const std::string &current)
    {
        std::optional<std::string> chosen;

        ImGui::TextUnformatted(label);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::BeginCombo(("##" + std::string(label)).c_str(), levelName(current).c_str()))
        {
            for (const std::string &path : levelPathsIn(directoryOf(current)))
            {
                bool isCurrent = path == current;
                if (ImGui::Selectable(levelName(path).c_str(), isCurrent) && !isCurrent)
                    chosen = path;
            }
            ImGui::EndCombo();
        }

        return chosen;
    }
}

void LevelsUi::draw(Levels &levels, const Level &level, EditorCommands &commands)
{
    if (std::optional<std::string> chosen = levelChooser("playing", level.getPath()))
        commands.onLoadLevel(*chosen);

    ImGui::Separator();

    saveable.drawControls(
        "levels",
        levels.getFirst(),
        [&levels] { levels.save(); },
        [&levels](const std::string &lastSeen) { levels.setFirst(lastSeen); });

    ImGui::Separator();

    if (std::optional<std::string> chosen = levelChooser("first", levels.getFirst()))
        levels.setFirst(*chosen);
}

void LevelsUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
