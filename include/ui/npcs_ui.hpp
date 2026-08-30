#pragma once

#include <memory>
#include <vector>

class Level;
class Npc;

class NpcsUi
{
public:
    void draw(const Level &level, const std::vector<std::unique_ptr<Npc>> &npcs) const;
};
