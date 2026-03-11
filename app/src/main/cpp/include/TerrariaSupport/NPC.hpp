#pragma once

#include "TerrariaBase.hpp"
#include "Projectile.hpp"
#include "Entity.hpp"
#include "Main.hpp"
#include "NPCSpawnParams.hpp"

#define INSTANCE_NPC_FIELD_LIST \
    X(int, type)           \
    X(bool, active)        \
    X(bool, friendly)      \
    X(bool, immortal)           \
    X(bool, townNPC)      \
    X(bool, dontTakeDamage)

#define STATIC_NPC_STATIC_FIELD \
    Y(float, waveKills)         \
    Y(int, waveNumber)

#define INSTANCE_NPC_METHOD_LIST \
    Z(void, SetDefaults)         \
    Z(void, StrikeNPC)

#define STATIC_NPC_METHOD_LIST \
    T(void, SpawnBoss)   \
    T(int, NewNPC)             \
    T(int, SpawnWOF)

class NPC : public TerrariaBase<NPC> {
private:
    NPC() : TerrariaBase(oxorany("Terraria"), oxorany("NPC")) {
    #define X(type, name) INIT_FIELD(name)
        INSTANCE_NPC_FIELD_LIST
    #undef X

    #define Y(type, name) INIT_FIELD(name)
        STATIC_NPC_STATIC_FIELD
    #define Y

    #define Z(returnType, name) INIT_METHOD(name)
        INSTANCE_NPC_METHOD_LIST
    #undef Z

    #define T(returnType, name) INIT_METHOD(name)
        STATIC_NPC_METHOD_LIST
    #undef T
    }
    friend class TerrariaBase<NPC>;

public:
    #define X(type, name) DECLARE_FIELD(type, name)
        INSTANCE_NPC_FIELD_LIST
    #undef X

    #define Y(type, name) DECLARE_FIELD(type, name)
        STATIC_NPC_STATIC_FIELD
    #undef Y

    #define Z(returnType, name) DECLARE_METHOD(returnType, name)
        INSTANCE_NPC_METHOD_LIST
    #undef Z

    #define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_NPC_METHOD_LIST
    #undef T

    #define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
        INSTANCE_NPC_FIELD_LIST
    #undef X

    #define Y(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
        STATIC_NPC_STATIC_FIELD
    #undef Y

    #define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
        INSTANCE_NPC_METHOD_LIST
    #undef Z

    #define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
        STATIC_NPC_METHOD_LIST
    #undef T

//    static void SpawnNPC(int type){
//        auto player = Main::getLocalPlayer();
//        auto npc = Instance()._class.CreateNewObjectParameters();
//        auto spawnParams = NPCSpawnParams::Instance()._class.CreateNewObjectParameters();
//        SetDefaults_Call((BNM::UnityEngine::Object*)npc, type, spawnParams);
//        auto current_type = gettype((BNM::UnityEngine::Object*)npc);
//        auto source = Projectile::GetNoneSource_Call();
//        auto pos = Entity::getposition(player);
//        auto index = NewNPC_Call(source, pos.x, pos.y, current_type, 0, 0, 0 ,0 ,0 ,255);
//    }

    static int SetDefaults(int type){
        auto feat = EventUpdateHandler::GetInstance().AddEventWithResult([type] () -> int {
                auto npc_Instance = Instance()._class.CreateNewObjectParameters();
                auto NPCSpawnParams = NPCSpawnParams::Instance()._class.CreateNewObjectParameters();
                NPC::Instance().SetDefaults_m[npc_Instance](type, NPCSpawnParams);
                return NPC::Instance().type_f[npc_Instance]();
        });
        return feat.get();
    }

    static void SpawnNPC(int x, int y, int type){
        auto feat = EventUpdateHandler::GetInstance().AddEventWithResult([x, y, type]() -> void {
            auto t = SetDefaults(type);
            auto source = Projectile::GetNoneSource_SyncCall();
            auto index = Instance().NewNPC_m(source, x, y, t, 0, 0, 0 ,0 ,0 ,255);
        });
    }

    static void SpawnBoss(int x, int y, int type) {
        auto t = SetDefaults(type);
        SpawnBoss_Call(x, y, t, Main::getmyPlayer(), 0, 0, 0, 0);
    }

    static void SpawnWOF(BNM::Structures::Unity::Vector2 pos){
        EventUpdateHandler::GetInstance().AddEventR([pos]() -> void {
            NPC::Instance().SpawnWOF_m(pos);
        });
    }

    static void killHostileNpcSync(){
        auto npcList = Main::getnpcSync()->ToVector();
        for (auto npc : npcList) {
            if(npc && getactiveSync(npc) && !gettownNPCSync(npc)){
                StrikeNPC_SyncCall(npc, INT32_MAX, 0, 0, false, false, false, -1);
            }
        }
    }
    static void KillHostileNpc(){
        EventUpdateHandler::GetInstance().AddEventR(killHostileNpcSync);
    }


    static void KillAllSync(){
        auto npcList = Main::getnpcSync()->ToVector();
        for (auto npc : npcList) {
            if(npc && getactiveSync(npc)){
                StrikeNPC_SyncCall(npc, INT32_MAX, 0, 0, false, false, false, -1);
            }
        }
    }

    static void KillAll(){
        EventUpdateHandler::GetInstance().AddEventR(KillAllSync);
    }
};