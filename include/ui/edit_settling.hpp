#pragma once

#include <optional>
#include <string>

class EditSettling
{
public:
    std::optional<std::string> settled(const std::string &now, bool stillBeingEdited);
    void startsAgainFrom(std::string now);

private:
    std::optional<std::string> whenItStarted;
    std::string lastSeen;
    bool looked = false;
};
