#pragma once

#include "game/npc/npc_data.hpp"
#include "game/actor/actor.hpp"

class Npc : public Actor
{
public:
    explicit Npc(const NpcData &data);
};
