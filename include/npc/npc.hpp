#pragma once

#include "npc/npc_data.hpp"
#include "actor/actor.hpp"

class Npc : public Actor
{
public:
    explicit Npc(const NpcData &data);
};
