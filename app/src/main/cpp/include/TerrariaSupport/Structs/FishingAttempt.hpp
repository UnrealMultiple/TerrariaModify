#pragma once

#include "BNM/UnityStructures.hpp"
#include "PlayerFishingConditions.hpp"

struct FishingAttempt
{
    PlayerFishingConditions playerFishingConditions;
    int32_t X;
    int32_t Y;
    int32_t bobberType;
    bool common;
    bool uncommon;
    bool rare;
    bool veryrare;
    bool legendary;
    bool crate;
    bool junk;
    bool inLava;
    bool inHoney;
    int32_t waterTilesCount;
    int32_t waterNeededToFish;
    float waterQuality;
    int32_t chumsInWater;
    int32_t fishingLevel;
    bool CanFishInLava;
    float atmo;
    int32_t questFish;
    int32_t heightLevel;
    int32_t rolledItemDrop;
    int32_t rolledEnemySpawn;
};