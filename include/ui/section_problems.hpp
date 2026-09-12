#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "ui/editor_section.hpp"

struct SectionProblem
{
    EditorSection section = EditorSection::Runtime;
    std::string_view name;
    std::string because;

    bool operator==(const SectionProblem &) const = default;
};

std::vector<SectionProblem> problemsAmong(std::span<const std::optional<std::string>> whyNotSaved);
