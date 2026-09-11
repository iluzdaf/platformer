#include <optional>
#include <string>
#include <utility>
#include "ui/edit_settling.hpp"

std::optional<std::string> EditSettling::settled(const std::string &now, bool stillBeingEdited)
{
    if (!looked)
    {
        startsAgainFrom(now);
        return std::nullopt;
    }

    if (now != lastSeen)
    {
        if (!whenItStarted)
            whenItStarted = lastSeen;

        lastSeen = now;
    }

    if (!whenItStarted || stillBeingEdited)
        return std::nullopt;

    std::optional<std::string> was;
    was.swap(whenItStarted);
    if (was == lastSeen)
        return std::nullopt;

    return was;
}

void EditSettling::startsAgainFrom(std::string now)
{
    whenItStarted.reset();
    lastSeen = std::move(now);
    looked = true;
}
