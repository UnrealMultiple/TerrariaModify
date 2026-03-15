#pragma once

#include "TerrariaBase.hpp"
#include "Main.hpp"
#include "NetMessage.hpp"

#define STATIC_WORLDGEN_FIELD \
    X(bool, spawnMeteor)      \
    X(int, meteorShowerCount)

#define STATIC_WORLDGEN_METHODS(T) \
    T(void, KillTile)              \
    T(void, KillWall)              \
    T(void, dropMeteor)            \
    T(void, StartMeteorShower)

#define STATIC_WORLDGEN_OVERLOADS(U) \
    U(bool, InWorld)

class WorldGen : public TerrariaBase<WorldGen> {
private:
    WorldGen() : TerrariaBase(oxorany("Terraria"), oxorany("WorldGen")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_WORLDGEN_METHODS(T)
#undef T

#define X(type, name) INIT_FIELD(name)
        STATIC_WORLDGEN_FIELD
#undef X

        InWorld_m = _class.GetMethod(oxorany("InWorld"), {oxorany("x"), oxorany("y"), oxorany("fluff")});
    }
    friend class TerrariaBase<WorldGen>;

public:
#define X(type, name) DECLARE_STATIC_FIELD(type, name)
    STATIC_WORLDGEN_FIELD
#undef X

#define X(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_WORLDGEN_FIELD
#undef X

#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T

#define U(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_WORLDGEN_OVERLOADS(U)
#undef U

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T

#define U(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_WORLDGEN_OVERLOADS(U)
#undef U

    static void KillCoordinatesTile(int x, int y, int radius, bool killwall = false) {
        auto netMode = Main::getnetModeSync();
        auto maxTilesX = Main::getmaxTilesXSync();
        auto maxTilesY = Main::getmaxTilesYSync();
        for (int i = x - radius; i <= x + radius; i++) {
            for (int j = y - radius; j <= y + radius; j++) {
                if (i >= 0 && i < maxTilesX && j >= 0 && j < maxTilesY) {
                    if(InWorld_SyncCall(i , j, 0)){
                        KillTile_SyncCall(i, j, false, false, false);
                        if(netMode == 1){
                            NetMessage::SendDataSync(PacketData{
                                    .msgType = 17,
                                    .number = 0,
                                    .number2 = (float)i,
                                    .number3 = (float)j
                            });
                        }
                        if(killwall) {
                            KillWall_SyncCall(i ,j , false);
                            if(netMode){
                                NetMessage::SendDataSync(PacketData{
                                        .msgType = 17,
                                        .number = 2,
                                        .number2 = (float)i,
                                        .number3 = (float)j
                                });
                            }
                        }
                    }
                }
            }
        }
    }
};