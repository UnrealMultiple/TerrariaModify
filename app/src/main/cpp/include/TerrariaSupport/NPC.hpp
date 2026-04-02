#pragma once

#include "TerrariaBase.hpp"
#include "Projectile.hpp"
#include "Entity.hpp"
#include "Main.hpp"
#include "NPCSpawnParams.hpp"
#include "NetMessage.hpp"
#include "UnifiedRandom.hpp"
#include "TileData.hpp"
#include "TerrariaSupport/Structs/Tile.hpp"
#include "WorldGen.hpp"

#define INSTANCE_NPC_FIELD_LIST \
    X(int, type)           \
    X(bool, active)        \
    X(bool, friendly)      \
    X(bool, immortal)           \
    X(bool, townNPC)      \
    X(bool, dontTakeDamage)     \
    X(int, life)                \
    X(int, defense)

#define STATIC_NPC_STATIC_FIELD \
    Y(float, waveKills)         \
    Y(int, waveNumber)

#define INSTANCE_NPC_METHOD_LIST \
    Z(void, SetDefaults)         \
    Z(void, StrikeNPC)           \
    Z(void, StrikeNPCNoInteraction)

#define STATIC_NPC_METHOD_LIST \
    T(void, SpawnBoss)   \
    T(int, NewNPC)             \
    T(int, SpawnWOF)           \
    T(BNM::UnityEngine::Object*, SpawnNPC)                           \
    T(BNM::UnityEngine::Object*, GetBossSpawnSource)

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

    #define T(returnType, name) DECLARE_METHOD(returnType, name)
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

    static int SetDefaults(int type){
        auto feat = EventUpdateHandler::GetInstance().AddEventWithResult([type] () -> int {
                auto npc_Instance = Instance()._class.CreateNewObjectParameters();
                auto NPCSpawnParams = NPCSpawnParams::Instance()._class.CreateNewObjectParameters();
                NPC::Instance().SetDefaults_m[npc_Instance](type, NPCSpawnParams);
                return NPC::Instance().type_f[npc_Instance]();
        });
        return feat.get();
    }

    static void GetRandomClearTileWithInRangeSync(int startTileX, int startTileY, int tileXRange, int tileYRange,
                                       int& tileX, int& tileY){
        auto tile = Main::gettileSync();
        int j = 0;
        do
        {
            if (j == 100)
            {
                tileX = startTileX;
                tileY = startTileY;
                break;
            }
            tileX = startTileX + UnifiedRandom::Next(tileXRange * -1, tileXRange);
            tileY = startTileY + UnifiedRandom::Next(tileYRange * -1, tileYRange);
            j++;
            LOGI("Tile id %d => %d", TileData::GetType(tileX, tileY), WorldGen::SolidTile_SyncCall(TileData::get_Item_SyncCall(tile, tileX, tileY)));
        } while (WorldGen::SolidTile_SyncCall(TileData::get_Item_SyncCall(tile, tileX, tileY)));
    }

    static bool TilePlacementValid(int tileX, int tileY){
        auto maxTileX = Main::getmaxTilesY();
        auto maxTileY = Main::getmaxTilesY();
        return tileX >= 0 && tileX < maxTileX && tileY >= 0 && tileY < maxTileY;
    }

    static void SpawnNPC(int x, int y, int type){
        auto feat = EventUpdateHandler::GetInstance().AddEventWithResult([x, y, type]() -> void {
            int newX, newY;
            GetRandomClearTileWithInRangeSync(x / 16, y / 16, 50, 20, newX, newY);
            auto EntitySource_DebugCommand =  BNM::Class("Terraria.DataStructures","EntitySource_DebugCommand");
            auto source = EntitySource_DebugCommand.CreateNewObjectParameters();
            NPC::NewNPC_SyncCall(source, newX * 16, newY * 16, type, 0, 0.0f, 0.0f ,0.0f ,0.0f, 255);
        });
    }

    static void SpawnBoss(int x, int y, int type) {
        auto playerIndex = Main::getmyPlayer();
        SpawnBoss_Call(x, y, type, playerIndex, 0, 0, 0, 0);
    }

    static void SpawnWOF(BNM::Structures::Unity::Vector2 pos){
        EventUpdateHandler::GetInstance().AddEventR([pos]() -> void {
            NPC::Instance().SpawnWOF_m(pos);
        });
    }

    static void ButcherAllHostileNPCsSync(int damage = 1000, int hitCount = -1){
        auto npcs = Main::getnpcSync()->ToVector();
        for (int i = 0; i < npcs.size(); i++){
            auto npc = npcs[i];
            if (NPC::getactiveSync(npc) && !NPC::getfriendlySync(npc) && NPC::gettypeSync(npc) != 388){
                ButcherNPCBypassTShockSync(npc, hitCount);
                ButcherNPCSync(npc, damage, hitCount);
            }
        }
    }

    static void ButcherNPCSync(BNM::UnityEngine::Object* npc, int damage = 1000, int hitCount = -1){
        int trueHitCount = hitCount;
        if (hitCount == -1){
            trueHitCount = (int)ceil((float)(getlifeSync(npc) + (int)ceil(getdefenseSync(npc) / 2.0f)) / damage);
        }
        for (int j = 0; j < trueHitCount; j++){
            StrikeNPCNoInteraction_SyncCall(npc, damage, 0.0f, 0, true, false, false);
            NetMessage::SendDataSync(PacketData{
                    .msgType = 28,
                    .number = Entity::getwhoAmISync(npc),
                    .number2 = (float)damage
            });
        }
    }

    static void ButcherNPCBypassTShockSync(BNM::UnityEngine::Object* npc, int hitCount = -1){
        if (getimmortalSync(npc))
            return;

        if (hitCount < 0)
            hitCount = (int)ceil((float) getlifeSync(npc) / SHRT_MAX);
        for (int i = 0; i < hitCount; i++){
            NetMessage::SendDataSync(PacketData{
                .msgType = 153,
                .number = Entity::getwhoAmISync(npc),
                .number2 = SHRT_MAX
            });
        }
    }

    static void ButcherAllNPCsSync(int damage = 1000, int hitCount = -1){
        ButcherAllHostileNPCsSync(damage, hitCount);
        ButcherAllFriendlyNPCsSync(damage, hitCount);
    }

    static void ButcherAllFriendlyNPCsSync(int damage = 1000, int hitCount = -1){
        auto npcs = Main::getnpcSync()->ToVector();
        for (int i = 0; i < npcs.size(); i++){
            auto npc = npcs[i];
            if (getactiveSync(npc) && getfriendlySync(npc) && gettypeSync(npc) == 388){
                ButcherNPCBypassTShockSync(npc, hitCount);
                ButcherNPCSync(npc, damage, hitCount);
            }
        }
    }

    static void ButcherAllNPCs(int damage = 1000, int hitCount = -1){
        EventUpdateHandler::GetInstance().AddEventR(ButcherAllNPCsSync, damage, hitCount);
    }

    static void ButcherAllFriendlyNPCs(int damage = 1000, int hitCount = -1){
        EventUpdateHandler::GetInstance().AddEventR(ButcherAllFriendlyNPCsSync, damage, hitCount);
    }

    static void ButcherAllHostileNPCs(int damage = 1000, int hitCount = -1){
        EventUpdateHandler::GetInstance().AddEventR(ButcherAllHostileNPCsSync, damage, hitCount);
    }
};