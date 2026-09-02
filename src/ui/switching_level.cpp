#include <optional>
#include <string>
#include "ui/switching_level.hpp"

SwitchingLevel switching(
    const std::optional<std::string> &chosen,
    bool levelHasUnsavedChanges,
    const std::optional<std::string> &waitingOn,
    bool switchPressed,
    bool cancelPressed)
{
    std::optional<std::string> held = levelHasUnsavedChanges ? waitingOn : std::nullopt;

    if (chosen && !levelHasUnsavedChanges)
        return {chosen, std::nullopt};

    if (chosen)
        return {std::nullopt, chosen};

    if (held && switchPressed)
        return {held, std::nullopt};

    if (held && cancelPressed)
        return {std::nullopt, std::nullopt};

    return {std::nullopt, held};
}
