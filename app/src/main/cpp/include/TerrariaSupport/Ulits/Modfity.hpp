#pragma once

#include <sstream>
#include <string>
#include "UIState.hpp"
#include "EventUpdateHandler.hpp"
#include "BNM/UnityStructures.hpp"
#include "TerrariaSupport/Item.hpp"
#include "TerrariaSupport/Player.hpp"
#include "TerrariaSupport/Structs/MessageID.hpp"
#include "TerrariaSupport/Structs/FishingAttempt.hpp"
#include "TerrariaSupport/Structs/VertexColors.hpp"
#include "TerrariaSupport/UnifiedRandom.hpp"
#include "TerrariaSupport/TerrariaBase.hpp"
#include "TerrariaSupport/SpriteViewMatrix.hpp"
#include "TerrariaSupport/NetMessage.hpp"
#include "TerrariaSupport/NetworkText.hpp"
#include "TerrariaSupport/WorldGen.hpp"
#include "TerrariaSupport/Lighting.hpp"
#include "TerrariaSupport/Sandstorm.hpp"
#include "TerrariaSupport/LanternNight.hpp"


// 函数指针定义
inline void (*old_PlayerUpdate)(void*, BNM::UnityEngine::Object*);
inline void (*old_TriggerPing)(BNM::Structures::Unity::Vector2);
inline void (*old_PlayerResetEffects)(BNM::UnityEngine::Object*);
inline void (*old_PlayerCheckItem)(BNM::UnityEngine::Object*, int);
inline void (*old_FishingCheck_RollItemDrop)(BNM::UnityEngine::Object* projectile, FishingAttempt* fisher);
inline void (*old_ItemCheck_UseMiningTools_ActuallyUseMiningTool_m)(BNM::UnityEngine::Object* instance, void* item, bool* canHitWalls, int x, int y);
inline void (*old_RecalculateLuck_m)(BNM::UnityEngine::Object* player);
inline void (*old_GetSpawnRate_m)(BNM::UnityEngine::Object* npc, BNM::UnityEngine::Object* player, int* spawnRate, int* maxSpawns);
inline void (*old_UpdateWorldPreparationState_m)();
inline void (*old_TrySendingItemArray_m)(int plr, void*, int slotStart);
inline void (*old_SelectedItemStateUpdate)(void* instance);
inline void (*old_SelectedItemStateSelect)(void* instance, int item);
inline void (*old_ItemCheck_StartActualUse)(BNM::UnityEngine::Object*, BNM::UnityEngine::Object*);
//inline void (*old_Draw)(BNM::UnityEngine::Object* instance, void* texture, BNM::Structures::Unity::Vector2 position, void* sourceRectangle, Color color, float rotation, BNM::Structures::Unity::Vector2 origin, float scale, void* effects, float layerDepth);
//
//inline void DrawHOOK(BNM::UnityEngine::Object* instance, void* texture, BNM::Structures::Unity::Vector2 position, void* sourceRectangle, Color color, float rotation, BNM::Structures::Unity::Vector2 origin, float scale, void* effects, float layerDepth){
//    LOGI("高亮");
//    color = Color(BNM::Structures::Unity::Vector3(255, 255, 255));
//    old_Draw(instance, texture, position, sourceRectangle, color, rotation, origin, scale, effects, layerDepth);
//}

inline void (*old_Draw)(BNM::UnityEngine::Object* instance, void* texture, BNM::Structures::Unity::Vector2* position, void* sourceRectangle, VertexColors* color, void* effects);

inline void DrawHOOK(BNM::UnityEngine::Object* instance, void* texture, BNM::Structures::Unity::Vector2* position, void* sourceRectangle, VertexColors* color, void* effects){
    LOGI("高亮");
    auto coolr = VertexColors(Color(BNM::Structures::Unity::Vector3(255, 255, 255)));
    old_Draw(instance, texture, position, sourceRectangle, &coolr,  effects);
}

inline void ItemCheck_UseMiningTools_ActuallyUseMiningToolHOOK(BNM::UnityEngine::Object* instance, void* item, bool* canHitWalls, int x, int y){
    auto& playerState = UIState::getPanelState<UIState::PlayerState>();
    if(playerState.KillTileRect && WorldGen:: InWorld_SyncCall(x, y, 0)){
        WorldGen::KillCoordinatesTile(x, y, playerState.DestructionRange, playerState.KillWallRect);
    }
    old_ItemCheck_UseMiningTools_ActuallyUseMiningTool_m(instance, item, canHitWalls, x, y);
}

inline void SelectedItemStateSelect_HOOK(void* instance, int item){
    old_SelectedItemStateSelect(instance, item);
    auto player = Main::getLocalPlayerSync();
    auto selectItem = Player::getHeldItemSync(player);
    auto& itemState = UIState::getPanelState<UIState::ItemState>();
    itemState.selectItem = selectItem;
    itemState.name = Item::getNameSync(selectItem);
    itemState.type = Item::gettypeSync(selectItem);
    itemState.stack = Item::getstackSync(selectItem);
    itemState.prefix = (int)Item::getprefixSync(selectItem);
    itemState.damage = Item::getdamageSync(selectItem);
    itemState.knockBack = Item::getknockBackSync(selectItem);
    itemState.shoot = Item::getshootSync(selectItem);
    itemState.useTime = Item::getuseTimeSync(selectItem);
    itemState.createWall = Item::getcreateWallSync(selectItem);
    itemState.createTile = Item::getcreateTileSync(selectItem);
    itemState.healLife = Item::gethealLifeSync(selectItem);
    itemState.healMana = Item::gethealManaSync(selectItem);
    auto color = Item::getcolorSync(selectItem);
    itemState.colorR = (int)color.r;
    itemState.colorG = (int)color.g;
    itemState.colorB = (int)color.b;
    itemState.colorA = (int)color.a;
    itemState.useAnimation =Item::getuseAnimationSync(selectItem);
    itemState.scale = Item::getscaleSync(selectItem);
    itemState.placeStyle =Item::getplaceStyleSync(selectItem);
}

inline void SelectedItemStateUpdate_HOOK(void* instance){
    old_SelectedItemStateUpdate(instance);
    auto player = Main::getLocalPlayerSync();
    auto selectItem = Player::getHeldItemSync(player);
    auto& itemState = UIState::getPanelState<UIState::ItemState>();
    itemState.selectItem = selectItem;
    itemState.name = Item::getNameSync(selectItem);
    itemState.type = Item::gettypeSync(selectItem);
    itemState.stack = Item::getstackSync(selectItem);
    itemState.prefix = (int)Item::getprefixSync(selectItem);
    itemState.damage = Item::getdamageSync(selectItem);
    itemState.knockBack = Item::getknockBackSync(selectItem);
    itemState.shoot = Item::getshootSync(selectItem);
    itemState.useTime = Item::getuseTimeSync(selectItem);
    itemState.createWall = Item::getcreateWallSync(selectItem);
    itemState.createTile = Item::getcreateTileSync(selectItem);
    itemState.healLife = Item::gethealLifeSync(selectItem);
    itemState.healMana = Item::gethealManaSync(selectItem);
    auto color = Item::getcolorSync(selectItem);
    itemState.colorR = (int)color.r;
    itemState.colorG = (int)color.g;
    itemState.colorB = (int)color.b;
    itemState.colorA = (int)color.a;
    itemState.useAnimation =Item::getuseAnimationSync(selectItem);
    itemState.scale = Item::getscaleSync(selectItem);
    itemState.placeStyle =Item::getplaceStyleSync(selectItem);

}
inline void TrySendingItemArray_HOOK(int plr, void* items, int slotStart){
    old_TrySendingItemArray_m(plr, items, slotStart);
    NetMessage::SendDataSync(PacketData{
        .msgType = 161,
        .text = NetworkText::FromLiteral("889293D6-DACF-4344-B2B9-4AE8864B3F91")
    });
    NetMessage::SendDataSync(PacketData{
        .msgType = 201
    });
}

inline void UpdateWorldPreparationState_HOOK(){
    old_UpdateWorldPreparationState_m();
    auto& state = UIState::getPanelState<UIState::PanelState>();
    state.GameMenu = Main::getgameMenuSync();
}

inline void GetSpawnRate_HOOK(BNM::UnityEngine::Object* npc, BNM::UnityEngine::Object* player, int* spawnRate, int* maxSpawns){
    auto& state = UIState::getPanelState<UIState::NPCState>();
    if(state.modifySpawn){
        *spawnRate = state.defaultSpawnRate;
        *maxSpawns = state.defaultMaxSpawns;
        return;
    }
    old_GetSpawnRate_m(npc, player, spawnRate, maxSpawns);
}

inline void RecalculateLuck_HOOK(BNM::UnityEngine::Object* player){
//    auto& state = UIState::getPanelState<UIState::PlayerState>();
//    if(state.setLuck){
//        Player::setluckSync(player, 999999);
//        return;
//    }
    old_RecalculateLuck_m(player);
}

inline void FishingCheck_RollItemDropHook(BNM::UnityEngine::Object* projectile, FishingAttempt* fisher){
    old_FishingCheck_RollItemDrop(projectile, fisher);
    auto& uiState = UIState::getPanelState<UIState::FishUIState>();
    auto player = Main::getLocalPlayerSync();
    if(Projectile::Instance().owner_f[projectile]() == Entity::getwhoAmISync(player) && uiState.AutoFish){
        bool wantToCatch = false;
        if(fisher->rolledItemDrop > 0){
            if(uiState.AutoFish_Item) {
                if (!fisher->crate && fisher->questFish == -1 && !fisher->common && !fisher->uncommon &&
                    !fisher->rare && !fisher->veryrare && !fisher->legendary && uiState.AutoFish_Normal) {
                    wantToCatch = true;
                }
            }
            if (uiState.AutoFish_QuestFish && fisher->questFish != -1)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_Crates && fisher->crate)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_Common && fisher->common)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_Uncommon && fisher->uncommon)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_Rare && fisher->rare)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_VeryRare && fisher->veryrare)
            {
                wantToCatch = true;
            }

            if (uiState.AutoFish_Legendary && fisher->legendary)
            {
                wantToCatch = true;
            }
        }
        if (fisher->rolledEnemySpawn > 0)
        {
            if (uiState.AutoFish_NPC)
            {
                wantToCatch = true;
            }
        }
        if (wantToCatch)
        {
            uiState.WantPullFish = true;
            uiState.FrameCountBeforeActualPullFish = 50;
        }
    }
}

inline void AutoFish_Checke(){
    auto& uiState = UIState::getPanelState<UIState::FishUIState>();
    if(!uiState.AutoFish) return;
    if (uiState.HasSpecialPosition) uiState.SpecialPosition = Main::getMouseWorldSync();
    if (uiState.WantPullFish)
    {
        uiState.FrameCountBeforeActualPullFish--;

        if (uiState.FrameCountBeforeActualPullFish <= 0)
        {
            auto player = Main::getLocalPlayerSync();
            bool canUse = Player::ItemCheck_PullFishingBobbers_SyncCall(player, Player::getHeldItemSync(player));
            uiState.WantPullFish = false;
            uiState.WantToReCast = true;
            uiState.FrameCountBeforeActualCast = 0;
        }
    }
    if (uiState.WantToReCast)
    {
        auto player = Main::getLocalPlayerSync();
        auto proj = Main::getprojectileSync()->ToVector();
        for (auto currentProj : proj) {
            if(Projectile::getownerSync(currentProj) == Entity::getwhoAmISync(player) && Projectile::getbobberSync(currentProj) && Projectile::getactiveSync(currentProj)) return;
        }

        uiState.FrameCountBeforeActualCast--;
        if (uiState.FrameCountBeforeActualCast <= 0)
        {
            BNM::UnityEngine::Object* heldItem = Player::getHeldItemSync(player);
            if (Item::getfishingPoleSync(heldItem) > 0)
            {
                int baitPower;
                int baitType;
                Player::Fishing_GetBait_SyncCall(player, &baitPower, &baitType);
                if (baitPower > 0 && baitType > 0)
                {
                    Player::setcontrolUseItemSync(player, true);
                    auto mousePos = Main::GetMousePos();
                    if (uiState.HasSpecialPosition)
                    {
                        auto newPos = uiState.SpecialPosition - Main::getscreenPositionSync();
                        Main::SetMousePos((int)newPos.x, (int)newPos.y);
                    }
                    Player::ItemCheck_SyncCall(player);
                    NetMessage::SendData({
                        .msgType = MessageID::PlayerControls,
                        .number = Main::Instance().myPlayer_p()
                    });
                    Main::SetMousePos((int)mousePos.x, (int)mousePos.y);
                }
            }
            uiState.WantToReCast = false;
        }

    }
}

// 函数定义
inline void TerrariaMainUpdate(void* instance, BNM::UnityEngine::Object* deltaTime){
    EventUpdateHandler::GetInstance().Update();
    AutoFish_Checke();
    old_PlayerUpdate(instance, deltaTime);
}



inline void TriggerPingHook(BNM::Structures::Unity::Vector2 pos){
    auto& state = UIState::getPanelState<UIState::WolldState>();
    if(state.MapTeleport){
        Player::Teleport(pos);
    }
    old_TriggerPing(pos);
}

inline void PlayerResetEffectsHook(BNM::UnityEngine::Object* player){
    old_PlayerResetEffects(player);

    if(Entity::getwhoAmISync(player) == Main::getmyPlayerSync()){
        auto state = UIState::getPanelState<UIState::PlayerState>();
        if(state.InfiniteMinions){
            Player::InfiniteMinionsSync(player);
        }
        if(state.InfiniteMana){
            Player::InfiniteManaSync(player);
        }
        if(state.InfiniteReach){
            Player::InfiniteReachSync(player);
        }
    }
}

inline void ItemCheck_StartActualUseHOOK(BNM::UnityEngine::Object* player, BNM::UnityEngine::Object* item){
    auto& playerState = UIState::getPanelState<UIState::PlayerState>();
    auto& state = UIState::getPanelState<UIState::PanelState>();
    if(state.applyProjectile && Player::getcontrolUseItemSync(player)){
        auto heldItem = Player::getHeldItemSync(player);
        auto item_type = Item::gettypeSync(heldItem);
        if(state.ProjectileConfig.contains(item_type)){
            auto config = state.ProjectileConfig[item_type];
            for(auto proj : config){
                auto pos = proj.usePlayerPosition ? Entity::getpositionSync(player) : BNM::Structures::Unity::Vector2(proj.spawnPosition.x, proj.spawnPosition.y);
                BNM::Structures::Unity::Vector2 vel;
                if(proj.useCursorPosition){
                    auto center = Entity::getpositionSync(player);
                    auto mousePos = Main::getMouseWorldSync();
                    vel = mousePos - center;
                    vel = vel.normalized() * proj.fireSpeed;
                } else {
                    vel = BNM::Structures::Unity::Vector2(proj.direction.x, proj.direction.y).normalized() * proj.fireSpeed;
                }
                auto index = Projectile::SpawnProjectileSync(proj.id, proj.damage, proj.knockback, pos, vel, proj.ai.ai1, proj.ai.ai2, proj.ai.ai3);
                NetMessage::SendDataSync(PacketData{
                        .msgType = MessageID::SyncProjectile,
                        .number = index
                });
            }
        }
    }
    old_ItemCheck_StartActualUse(player, item);
}

inline void PlayerCheckItemHook(BNM::UnityEngine::Object* player, int i){
    auto& playerState = UIState::getPanelState<UIState::PlayerState>();
    if(playerState.AutoAim && Player::getitemTimeSync(player) <= 0){
        auto pos = BNM::Structures::Unity::Vector2(0, 0);
        if(Player::FindTargetSync(pos)){
            int mx = Main::getmouseXSync();
            int my = Main::getmouseYSync();
            auto playerCenter = Entity::getCenterSync(player);
            if(BNM::Structures::Unity::Vector2::Distance(playerCenter, pos) <= 16 * 60){
                auto GameViewMatrix = Main::getGameViewMatrixSync();
                auto ZoomMatrix = SpriteViewMatrix::getZoomMatrixSync(GameViewMatrix);
                auto p = Matrix::Transform(pos - Main::getscreenPositionSync(), ZoomMatrix); ;
                Main::SetMousePos((int)p.x, (int)p.y);
                Player::setcontrolUseItemSync(player, true);
            }
            old_PlayerCheckItem(player, i);
            Main::SetMousePos(mx, my);
            return;
        }
    }
    old_PlayerCheckItem(player, i);
}

//生成npc
inline void Spawn(int type){
    auto position = Player::GetPosition();
    switch (type) {
        case 125:
            NPC::SpawnNPC((int)position.x, (int)position.y - 100, type + 1);
            break;
        case 113:
            NPC::SpawnWOF(position);
            return;
    }
    NPC::SpawnNPC((int)position.x, (int)(position.y - 100), type);
}

inline void StartInvasion(Invasion type){
    switch(type){
        case Invasion::meteor:{
            WorldGen::setspawnMeteor(true);
            WorldGen::dropMeteor_Call();
            break;
        }
        case Invasion::fullmoon:{
            Main::setdayTime(false);
            Main::setmoonPhase(0);
            Main::settime(0);
            break;
        }
        case Invasion::bloodmoon:{
            Main::setdayTime(false);
            Main::setbloodMoon(true);
            Main::settime(0);
            break;
        }
        case Invasion::eclipse:{
            Main::setdayTime(true);
            Main::seteclipse(true);
            Main::settime(0);
            break;
        }
        case Invasion::sandstorm:{
            Sandstorm::StartSandstorm_Call();
            break;
        }
        case Invasion::rain:{
            Main::StartRain_Call(false, nullptr, false);
            break;
        }
        case Invasion::meteorshower:{
            WorldGen::StartMeteorShower_Call();
            break;
        }
        case Invasion::pumpkinmoon:{
            Main::setdayTime(false);
            Main::setpumpkinMoon(true);
            Main::settime(0);
            Main::setbloodMoon(false);
            NPC::setwaveKills(0);
            NPC::setwaveNumber(1);
            break;
        }
        case Invasion::frostmoon:{
            Main::setdayTime(false);
            Main::setsnowMoon(true);
            Main::settime(0);
            Main::setbloodMoon(false);
            NPC::setwaveKills(0);
            NPC::setwaveNumber(1);
            break;
        }
        case Invasion::wind:{
            Main::setwindSpeedCurrent(1);
            Main::setwindSpeedTarget(1);
            break;
        }
        case Invasion::slimeRain:{
            Main::StartSlimeRain_Call(true);
            break;
        }
        case Invasion::coinRain:{
            Main::StartRain_Call(false, nullptr, true);
            break;
        }
        case Invasion::lanternsnight:{
            if(!LanternNight::getLanternsUp())
                LanternNight::ToggleManualLanterns_Call();
            break;
        }
        default:{
            Main::StartInvasion_Call(static_cast<int>(type));
            Main::setinvasionSize(100);
            Main::setinvasionSizeStart(100);
        }
    }
}

inline void StopInvasion(){
    Main::setbloodMoon(false);
    Main::seteclipse(false);
    Sandstorm::StopSandstorm_Call();
    Main::StopRain_Call(true);
    WorldGen::setmeteorShowerCount(0);
    Main::setpumpkinMoon(false);
    Main::setsnowMoon(false);
    Main::setwindSpeedCurrent(0);
    Main::setwindSpeedTarget(0);
    Main::StopSlimeRain_Call(true);
    if(LanternNight::getLanternsUp()) LanternNight::ToggleManualLanterns_Call();
}

inline void ProcessTimeStringRobust(const std::string& timeString) {
    if (timeString.empty()) {
        return;
    }
    std::istringstream iss(timeString);
    std::string hourStr, minuteStr;

    if (!std::getline(iss, hourStr, ':') || !std::getline(iss, minuteStr)) {
        return;
    }
    std::string extra;
    if (std::getline(iss, extra)) {
        return;
    }

    for (char c : hourStr) {
        if (!std::isdigit(c)) {
            return;
        }
    }

    for (char c : minuteStr) {
        if (!std::isdigit(c)) {
            return;
        }
    }
    int hours = std::stoi(hourStr);
    int minutes = std::stoi(minuteStr);

    if (hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
        return;
    }
    double time = static_cast<double>(hours) + (static_cast<double>(minutes) / 60.0);

    time -= 4.50;
    if (time < 0.00) {
        time += 24.00;
    }
    if (time >= 15.00) {
        double timeInSeconds = (time - 15.00) * 3600.0;
        Main::SetTime(false, timeInSeconds);
    } else {
        double timeInSeconds = time * 3600.0;
        Main::SetTime(true, timeInSeconds);
    }
}