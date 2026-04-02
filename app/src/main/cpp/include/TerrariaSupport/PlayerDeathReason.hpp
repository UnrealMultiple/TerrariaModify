#pragma once

#include "TerrariaBase.hpp"
#include "Main.hpp"
#include "NetMessage.hpp"

#define STATIC_PLAYER_DEATH_RESPASON_METHODS(T) \
    T(BNM::UnityEngine::Object*, ByPlayer)

class PlayerDeathReason : public TerrariaBase<PlayerDeathReason> {
private:
    PlayerDeathReason() : TerrariaBase(oxorany("Terraria.DataStructures"), oxorany("PlayerDeathReason")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_PLAYER_DEATH_RESPASON_METHODS(T)
#undef T
    }
    friend class TerrariaBase<PlayerDeathReason>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_PLAYER_DEATH_RESPASON_METHODS(T)
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_PLAYER_DEATH_RESPASON_METHODS(T)
#undef T
};