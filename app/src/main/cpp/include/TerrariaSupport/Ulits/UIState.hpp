#pragma once

#include "BNM/UnityStructures.hpp"
#include "TerrariaSupport/Enum/Invasion.hpp"
#include "menu/Projectile_Parsers.hpp"
#include <string>
#include <vector>

namespace UIState {
    enum PanelMode{
        showPlayer = 0,
        showWorld = 1,
        showNPC = 2,
        showItemGenerator = 3
    };

    struct PanelState{
        PanelMode State;
        bullet_config::BulletConfig ProjectileConfig;
        bool applyProjectile;
        bool GameMenu = true;
    };

    struct PlayerState{
        bool GodMode = false;
        bool FullBright = false;
        bool InfiniteMinions = false;
        bool InfiniteMana = false;
        bool InfiniteReach = false;
        bool KillTileRect = false;
        bool KillWallRect = false;
        bool AutoAim = false;
        bool AutoButcherPlayer = false;
        int DestructionRange = 40;
        bool ghost = false;
        int ghostSpeed = 25;
        int ghostIndex = -1;
        bool bestow = false;
        bool AllowTeleport = false;
        bool respawn = false;
        bool interceptRespawnPack = false;
        int respawnSecond = 15;
        int FramesSinceLastLifePacket = 0;
    };

    struct FishUIState{
        bool AutoFish;                                   //自动钓鱼
        bool AutoFish_Item;                              //接受物品
        bool AutoFish_NPC ;                               //接受npc
        bool AutoFish_Crates;                            //接受箱子
        bool AutoFish_Common ;                            //接受常见
        bool AutoFish_Uncommon ;                          //接受不常见
        bool AutoFish_Rare;                              //接受稀有
        bool AutoFish_VeryRare;                          //接受非常稀有
        bool AutoFish_Legendary;                         //接受传奇
        bool AutoFish_QuestFish ;                         //接受任务鱼
        bool AutoFish_Normal;                            //接受普通
        bool AutoFish_Junk;
        bool WantPullFish;                               //可以钓鱼
        bool WantToReCast;                               //可以重置
        bool HasSpecialPosition;                         //使用当前光标位置
        int FrameCountBeforeActualCast;
        int FrameCountBeforeActualPullFish;
        BNM::Structures::Unity::Vector2 SpecialPosition;         //保存光标位置
    };

    struct ItemState{
        BNM::UnityEngine::Object* selectItem = nullptr;
        BNM::Structures::Mono::String* name = nullptr;
        int type = 0;
        int prefix = 0;
        int stack = 0;
        int damage = 0;
        float knockBack = 0;
        int useTime = 0;
        int shoot = 0;
        int createTile = 0;
        int createWall = 0;
        int healLife = 0;
        int healMana = 0;
        int useAnimation = 0;
        int placeStyle = 0;
        float scale = 0;
        int colorR = 0;
        int colorG = 0;
        int colorB = 0;
        int colorA = 0;
    };

    struct NPCState{
        int defaultSpawnRate = 600;
        int defaultMaxSpawns = 5;
        int AutoButcherNPC = false;
        bool modifySpawn = false;
    };

    struct WolldState {
        bool MapTeleport = false;
        Invasion WorldInvasion = Invasion::goblin;
        std::vector<char> mapLoad;
        std::atomic<int> loadedSections{0};
        int totalSections = 0;
        int sectionsX = 0;
        int sectionsY = 0;
    };

    template<class T>
    T& getPanelState();

    extern template PlayerState& getPanelState<PlayerState>();
    extern template PanelState& getPanelState<PanelState>();
    extern template ItemState& getPanelState<ItemState>();
    extern template FishUIState& getPanelState<FishUIState>();
    extern template NPCState& getPanelState<NPCState>();
    extern template WolldState& getPanelState<WolldState>();
}