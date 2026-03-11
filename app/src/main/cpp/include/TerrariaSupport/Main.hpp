#pragma once

#include "TerrariaBase.hpp"
#include "Ulits/EventUpdateHandler.hpp"


#define MAIN_STATIC_FIELD_LIST \
    S(bool, dayTime) \
    S(double, time) \
    S(BNM::UnityEngine::Object*, tile) \
    S(BNM::Structures::Mono::Array<BNM::UnityEngine::Object*>*, npc) \
    S(BNM::UnityEngine::Object*, rand) \
    S(BNM::UnityEngine::Object*, GameViewMatrix)    \
    S(BNM::Structures::Mono::Array<BNM::UnityEngine::Object*>*, projectile) \
    S(BNM::UnityEngine::Object*, instance) \
    S(int, netMode) \
    S(float, GameZoomTarget)   \
    S(int, maxTilesX)          \
    S(int, maxTilesY)          \
    S(int, invasionSizeStart)  \
    S(int, invasionSize)       \
    S(int, moonPhase)          \
    S(bool, bloodMoon)         \
    S(bool, eclipse)           \
    S(bool, pumpkinMoon)       \
    S(bool, snowMoon)          \
    S(float, windSpeedCurrent) \
    S(float, windSpeedTarget)

#define MAIN_STATIC_PROPERTY_LIST \
    P(BNM::UnityEngine::Object*, LocalPlayer) \
    P(BNM::UnityEngine::Object*, Map) \
    P(bool, refreshMap) \
    P(int, screenWidth) \
    P(int, screenHeight) \
    P(int, myPlayer) \
    P(int, mouseX) \
    P(int, mouseY) \
    P(BNM::Structures::Unity::Vector2, MouseWorld) \
    P(bool, gameMenu) \
    P(BNM::Structures::Unity::Vector2, screenPosition) \
    P(BNM::UnityEngine::Object*, Achievements)

#define MAIN_STATIC_METHOD_LIST \
    M(void, Update) \
    M(void, TriggerPing) \
    M(void, DoDraw) \
    M(void, DrawBlack) \
    M(void, UpdateWorldPreparationState) \
    M(void, StartClientGameplay)    \
    M(void, StartInvasion)      \
    M(void, StartRain)          \
    M(void, StopRain)           \
    M(void, StartSlimeRain)     \
    M(void, StopSlimeRain)

class Main : public TerrariaBase<Main> {
private:
    Main() : TerrariaBase(oxorany("Terraria"), oxorany("Main")) {
#define S(type, name) INIT_FIELD(name)
        MAIN_STATIC_FIELD_LIST
#undef S

#define P(type, name) INIT_PROPERTY(name)
        MAIN_STATIC_PROPERTY_LIST
#undef P

#define M(returnType, name) INIT_METHOD(name)
        MAIN_STATIC_METHOD_LIST
#undef M
    }
    friend class TerrariaBase<Main>;

public:

#define S(type, name) DECLARE_STATIC_FIELD(type, name)
    MAIN_STATIC_FIELD_LIST
#undef S

#define P(type, name) DECLARE_STATIC_PROPERTY(type, name)
    MAIN_STATIC_PROPERTY_LIST
#undef P

#define M(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    MAIN_STATIC_METHOD_LIST
#undef M

#define S(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    MAIN_STATIC_FIELD_LIST
#undef S

#define P(type, name) DEFINE_STATIC_PROPERTY_ACCESSORS(name, type, name##_p)
    MAIN_STATIC_PROPERTY_LIST
#undef P

#define M(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    MAIN_STATIC_METHOD_LIST
#undef M

    static void UnLockAchievements(){
        EventUpdateHandler::GetInstance().AddEventR([] () -> void {
            auto AchievementManager_cls = BNM::Class(oxorany("Terraria.Achievements"), oxorany("AchievementManager"));
            auto Achievement_cls = BNM::Class(oxorany("Terraria.Achievements"), oxorany("Achievement"));
            auto AchievementCondition_cls = BNM::Class(oxorany("Terraria.Achievements"), oxorany("AchievementCondition"));
            BNM::Method<BNM::Structures::Mono::List<BNM::UnityEngine::Object*>*> CreateAchievementsList_m = AchievementManager_cls.GetMethod(oxorany("CreateAchievementsList"));
            BNM::Property<bool> IsCompleted_p = Achievement_cls.GetProperty(oxorany("IsCompleted"));
            BNM::Field<BNM::Structures::Mono::Dictionary<BNM::Structures::Mono::String*, BNM::UnityEngine::Object*>*> _conditions_p = Achievement_cls.GetField(oxorany("_conditions"));
            BNM::Method<void> Save_m = AchievementManager_cls.GetMethod(oxorany("Save"));
            BNM::Property<bool> IsCompleted2_p = AchievementCondition_cls.GetProperty(oxorany("IsCompleted"));
            BNM::Method<void> Complete_m = AchievementCondition_cls.GetMethod(oxorany("Complete"));
            auto manager = Main::getAchievementsSync();
            auto list = CreateAchievementsList_m[manager]();
            for (auto achievement  : list->ToVector()) {
                if(!IsCompleted_p[achievement ]()){
                    for(auto condition  : _conditions_p[achievement]()->GetValues()){
                        if(!IsCompleted2_p[condition]()){
                            Complete_m[condition]();
                        }
                    }
                }
            }
            Save_m[manager]();
        });
    }

    static void LigthMap(){
        EventUpdateHandler::GetInstance().AddEventR([] () -> void{
            auto TileData_cls = BNM::Class(oxorany("Terraria"),oxorany("TileData"));
            BNM::Field<int> width_f = TileData_cls.GetField(oxorany("_width"));
            BNM::Field<int> heigth_f = TileData_cls.GetField(oxorany("_height"));
            auto width = width_f[Main::Instance().tile_f()]();
            auto heigth = heigth_f[Main::Instance().tile_f()]();
            auto WorldMap_cls = BNM::Class(oxorany("Terraria.Map"),oxorany("WorldMap"));
            BNM::Method<void> Update = WorldMap_cls.GetMethod(oxorany("Update"));
            BNM::Method<void> Load = WorldMap_cls.GetMethod(oxorany("Load"));
            for (int w = 0; w < width; ++w) {
                for (int i = 0; i < heigth; ++i) {
                    Update[Main::Instance().Map_p()](w,i,255);
                }
            }
            setrefreshMapSync(true);
        });
    }

    static void SetTime(bool dayTime, double time){
        setdayTimeSync(dayTime);
        settimeSync(time);
    }

    static void SetMousePos(int x, int y){
        setmouseXSync(x);
        setmouseYSync(y);
    }

    static BNM::Structures::Unity::Vector2 GetMousePos(){
        auto x = getmouseXSync();
        auto y = getmouseYSync();
        return BNM::Structures::Unity::Vector2((float)x, (float)y);
    }
};