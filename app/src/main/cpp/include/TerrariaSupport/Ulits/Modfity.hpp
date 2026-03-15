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
#include "TerrariaSupport/Lighting.hpp"
#include "TerrariaSupport/TileDrawing.hpp"


// 函数指针定义
inline void (*old_MainUpdate)(void*, BNM::UnityEngine::Object*);
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
inline void (*old_DrawBlack)(BNM::UnityEngine::Object* instance, bool force);
inline void (*old_TileDraw)(BNM::UnityEngine::Object* instance,  BNM::Structures::Unity::Vector2 screenPosition, BNM::Structures::Unity::Vector2 offset, int tileX, int TileY, void* tileDraw);
inline void (*old_PlayerUpdate)(BNM::UnityEngine::Object* player, int i);
inline BNM::Structures::Unity::Vector2 (*old_TileCollision)(BNM::UnityEngine::Object* player, BNM::Structures::Unity::Vector2 position, BNM::Structures::Unity::Vector2 velocity, bool FallThrough, bool IgnorePlats);
inline void (*old_SlopeDownMovement)(void* player);
inline void (*old_DryCollision)(void* player, BNM::Structures::Unity::Vector2 velocity, bool canFallThrough, bool ignorePlats);

inline void DryCollisionHOOK(BNM::UnityEngine::Object* player, BNM::Structures::Unity::Vector2 velocity, bool canFallThrough, bool ignorePlats){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(state.ghost){
        velocity.y = 0;
        return;
    }
    old_DryCollision(player, velocity, canFallThrough, ignorePlats);
}

inline void SlopeDownMovementHOOK(void* player){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(!state.ghost){
        old_SlopeDownMovement(player);
    }
}

inline BNM::Structures::Unity::Vector2 TileCollisionHOOK(BNM::UnityEngine::Object* player, BNM::Structures::Unity::Vector2 position, BNM::Structures::Unity::Vector2 velocity, bool fallThrough, bool ignorePlats){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(state.ghost){
        return velocity;
    }
    return old_TileCollision(player, position, velocity, fallThrough, ignorePlats);
}

inline void Ghost(BNM::UnityEngine::Object* player){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(state.ghost){
        auto pos = Entity::getpositionSync(player);
        auto move = BNM::Structures::Unity::Vector2(0, 0);
        Entity::setoldPositionSync(player, pos);
        float currentSpeed = 10;
        if(Player::getcontrolLeftSync(player)) move.x -= 1;
        if(Player::getcontrolRightSync(player)) move.x += 1;
        if(Player::getcontrolUpSync(player)) move.y -= 1;
        if(Player::getcontrolDownSync(player)) move.y += 1;
        pos += move *currentSpeed;
        Entity::setpositionSync(player, pos);
        Entity::setvelocitySync(player, move * currentSpeed);
    }
}

inline void PlayerUpdateHook(BNM::UnityEngine::Object* player, int i){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(state.ghost){
        Player::setmaxFallSpeedSync(player, 0);
        Player::setgravitySync(player, 0);
        Ghost(player);
    }
    old_PlayerUpdate(player, i);
}



inline void TileDrawHOOK(BNM::UnityEngine::Object* instance,  BNM::Structures::Unity::Vector2 screenPosition, BNM::Structures::Unity::Vector2 offset, int tileX, int tileY, void* tileDraw){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(state.FullBright){
        int l = 1;
        Lighting::AddLight_SyncCall(tileX, tileY, l, l, l);
    }
    old_TileDraw(instance,screenPosition, offset, tileX, tileY, tileDraw);
}

inline void DrawBlackHOOK(BNM::UnityEngine::Object* instance, bool force){
    auto& state = UIState::getPanelState<UIState::PlayerState>();
    if(!state.FullBright){
        old_DrawBlack(instance, force);
    }
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
    //Ghost();
    old_MainUpdate(instance, deltaTime);
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

inline void FullLigth(bool check){
    auto& playerState = UIState::getPanelState<UIState::PlayerState>();
    if(check){
        Lighting::setMode(2);
    }
    playerState.FullBright = check;
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
    Main::setinvasionSize(0);
    Main::setinvasionSizeStart(0);
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