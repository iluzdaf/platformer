#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include "ui/section_problems.hpp"
#include "ui/editor_section.hpp"

std::vector<SectionProblem> problemsAmong(std::span<const std::optional<std::string>> whyNotSaved)
{
    std::vector<SectionProblem> problems;
    for (std::size_t at = 0; at < whyNotSaved.size() && at < EditorSections.size(); ++at)
    {
        const std::optional<std::string> &why = whyNotSaved[at];
        if (!why)
            continue;

        const auto &[section, name] = EditorSections[at];
        problems.push_back(SectionProblem{section, name, *why});
    }

    return problems;
}
