#pragma once

#include "TerrariaBase.hpp"
#include "Projectile.hpp"
#include "Entity.hpp"
#include "Main.hpp"


class NPCSpawnParams : public TerrariaBase<NPCSpawnParams> {
private:
    NPCSpawnParams() : TerrariaBase("Terraria", "NPCSpawnParams") {

    }
    friend class TerrariaBase<NPCSpawnParams>;

};