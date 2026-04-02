#pragma once

#include "TerrariaBase.hpp"
#include "PlayerDeathReason.hpp"

#define INSTANCE_PLAYER_FIELD_LIST \
    X(int, maxMinions)   \
    X(bool, active)      \
    X(int, statLife)     \
    X(int, statLifeMax)  \
    X(int, statLifeMax2) \
    X(int, statMana)     \
    X(int, statManaMax)  \
    X(int, statManaMax2) \
    X(int, manaCost)     \
    X(int, maxTurrets)   \
    X(bool, controlUseItem)        \
    X(int, itemTime)    \
    X(float, luck)                 \
    X(int, itemAnimation)          \
    X(bool, releaseUseItem)        \
    X(bool, controlLeft)           \
    X(bool, controlRight)           \
    X(bool, controlUp)           \
    X(bool, controlDown)           \
    X(float, gravity)              \
    X(bool, noFallDmg)              \
    X(float, maxFallSpeed)         \
    X(bool, hostile)               \
    X(int, statDefense)

#define INSTANCE_PLAYER_PROPERTY_LIST \
    Y(int, tileRangeX)       \
    Y(int, tileRangeY)       \
    Y(BNM::UnityEngine::Object*, HeldItem)

#define INSTANCE_PLAYER_METHOD_LIST \
    Z(void, RecalculateLuck) \
    Z(void, Teleport) \
    Z(void, ItemCheckWrapped) \
    Z(void, ResetEffects) \
    Z(bool, ItemCheck_PullFishingBobbers) \
    Z(void, Fishing_GetBait)\
    Z(void, ItemCheck)              \
    Z(void, ItemCheck_UseMiningTools_ActuallyUseMiningTool) \
    Z(void, AddBuff)                \
    Z(BNM::UnityEngine::Object*, GetProjectileSource_Item)  \
    Z(void, ItemCheck_StartActualUse)     \
    Z(void, SlopeDownMovement)      \
    Z(void, TileCollision)          \
    Z(void, DryCollision)           \
    Z(void, WetCollision)           \
    Z(void, Update)                 \
    Z(void, KillMe)                 \
    Z(void, GetRespawnTime)         \
    Z(void, SlopingCollision)       \
    Z(double, Hurt)

class Player : public TerrariaBase<Player> {
private:
    Player() : TerrariaBase(oxorany("Terraria"), oxorany("Player")) {
        #define X(type, name) INIT_FIELD(name)
                INSTANCE_PLAYER_FIELD_LIST
        #undef X

        #define Y(type, name) INIT_PROPERTY(name)
                INSTANCE_PLAYER_PROPERTY_LIST
        #undef Y

        #define Z(returnType, name) INIT_METHOD(name)
                INSTANCE_PLAYER_METHOD_LIST
        #undef Z

    }
    friend class TerrariaBase<Player>;

public:
    #define X(type, name) DECLARE_FIELD(type, name)
        INSTANCE_PLAYER_FIELD_LIST
    #undef X

    #define Y(type, name) DECLARE_PROPERTY(type, name)
        INSTANCE_PLAYER_PROPERTY_LIST
    #undef Y

    #define Z(returnType, name) DECLARE_METHOD(returnType, name)
        INSTANCE_PLAYER_METHOD_LIST
    #undef Z


    #define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
        INSTANCE_PLAYER_FIELD_LIST
    #undef X

    #define Y(type, name) DEFINE_INSTANCE_PROPERTY_ACCESSORS(name, type, name##_p)
        INSTANCE_PLAYER_PROPERTY_LIST
    #undef Y

    #define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
        INSTANCE_PLAYER_METHOD_LIST
    #undef Z

    static void ButcherAllPlayerSync(int damage = 1000, int hitCount = -1){
        auto players = Main::getplayerSync()->ToVector();
        for (auto player : players){
            if(Entity::getwhoAmISync(player) != Main::getmyPlayerSync() && Player::gethostileSync(player)){
                ButcherPlayer(player, damage, hitCount);
            }
        }
    }

    static void ButcherPlayer(BNM::UnityEngine::Object* player, int damage = 1000, int hitCount = -1){
        int trueHitCount = hitCount;
        if (hitCount == -1)
        {
            trueHitCount = (int)ceil((float)(Player::getstatLifeSync(player) + Player::getstatDefenseSync(player) / 2) / (float)damage);
        }
        for (int j = 0; j < trueHitCount; j++)
        {
            NetMessage::SendPlayerDeath_SyncCall(Entity::getwhoAmISync(player), PlayerDeathReason::ByPlayer_SyncCall(Main::getmyPlayerSync()), damage, 0, true, -1, -1);
        }
        NetMessage::SendDataSync(PacketData{
            .msgType = 13,
            .number = Main::getmyPlayerSync()
        });
    }

    static void GodMod(bool enabled){
        EventUpdateHandler::GetInstance().AddEventR([enabled] () -> void{
            auto CreativePowers = BNM::Class(oxorany("Terraria.GameContent.Creative"), oxorany("CreativePowers"));
            auto APerPlayerTogglePower = CreativePowers.GetInnerClass(oxorany("APerPlayerTogglePower"));
            BNM::Method<void> SetEnabledState = APerPlayerTogglePower.GetMethod(oxorany("SetEnabledState"));
            auto object = CreativePowers.GetInnerClass(oxorany("GodmodePower"));

            auto CreativePowerManager = BNM::Class(oxorany("Terraria.GameContent.Creative"), oxorany("CreativePowerManager"));
            BNM::Property<BNM::UnityEngine::Object*> instance = CreativePowerManager.GetProperty(oxorany("Instance"));

            auto method = CreativePowerManager.GetMethod(oxorany("GetPower"));
            BNM::Method<BNM::UnityEngine::Object*> GetPower = method.GetGeneric({object});
            auto index = Entity::getwhoAmISync(Main::getLocalPlayerSync());
            auto ins1 = GetPower[instance()]();
            SetEnabledState[ins1].Call(index, enabled);
        });
    }

    static void Teleport(BNM::Structures::Unity::Vector2 pos, int style = 1, int extraInfo = 0) {
        EventUpdateHandler::GetInstance().AddEventR([pos, style, extraInfo]() -> void {
            auto player = Main::getLocalPlayerSync();
            Instance().Teleport_m[player].Call(
                    BNM::Structures::Unity::Vector2(pos.x * 16, pos.y * 16), style, extraInfo);
        });
    }

    static void AddBUff(int type, int time ){
        EventUpdateHandler::GetInstance().AddEventR(AddBuffSync, type, time);
    }

    static bool FindTargetSync(BNM::Structures::Unity::Vector2& center){
        auto npcList = Main::getnpcSync()->ToVector();
        for (auto npc : npcList) {
            //if(!npc) continue;
            if(NPC::Instance().getactiveSync(npc) && !NPC::getfriendlySync(npc) && !NPC::getimmortalSync(npc) && !NPC::getdontTakeDamageSync(npc)){
                center = Entity::getCenterSync(npc);
                return true;
            }
        }
        return false;
    }

    static BNM::Structures::Unity::Vector2 GetPosition() {
        auto f = EventUpdateHandler::GetInstance().AddEventR(GetPositionSync);
        return f.get();
    }

    static BNM::Structures::Unity::Vector2 GetPositionSync(){
        auto player = Main::getLocalPlayerSync();
        return Entity::getposition(player);
        }

    static void AddBuffSync(int type, int time ){
        auto player = Main::getLocalPlayerSync();
        AddBuff_SyncCall(player, type, time, false);
    }

    static void SetMaxMinions(BNM::UnityEngine::Object* player){
        EventUpdateHandler::GetInstance().AddEventR(InfiniteMinionsSync, player);
    }

    static void InfiniteMinionsSync(BNM::UnityEngine::Object* player){
        Player::Instance().maxMinions_f[player].Set(INT32_MAX - 100000);
        Player::Instance().maxTurrets_f[player].Set(INT32_MAX - 100000);
    }

    static void InfiniteManaSync(BNM::UnityEngine::Object* player){
        Player::Instance().statMana_f[player].Set(Player::Instance().statManaMax2_f[player]());
        Player::Instance().manaCost_f[player].Set(0);
    }

    static void InfiniteMana(BNM::UnityEngine::Object* player){
        EventUpdateHandler::GetInstance().AddEventR(InfiniteMana, player);
    }

    static void InfiniteReachSync(BNM::UnityEngine::Object* player){
        Player::Instance().tileRangeX_p[player].Set(Main::Instance().screenWidth_p.Get() / 32 + 8);
        Player::Instance().tileRangeY_p[player].Set(Main::Instance().screenHeight_p.Get() / 32 + 8);
    }

    static void InfiniteReach(BNM::UnityEngine::Object* player){
        EventUpdateHandler::GetInstance().AddEventR(InfiniteReachSync, player);
    }

};