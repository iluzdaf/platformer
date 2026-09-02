#pragma once

#include <optional>
#include <string>

struct SwitchingLevel
{
    std::optional<std::string> loadNow;
    std::optional<std::string> waitingOn;

    bool operator==(const SwitchingLevel &) const = default;
};

SwitchingLevel switching(
    const std::optional<std::string> &chosen,
    bool levelHasUnsavedChanges,
    const std::optional<std::string> &waitingOn,
    bool switchPressed,
    bool cancelPressed);
