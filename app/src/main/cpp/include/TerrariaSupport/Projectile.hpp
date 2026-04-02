#pragma once

#include "TerrariaBase.hpp"
#include "Main.hpp"
#include "Entity.hpp"

#define INSTANCE_PROJECTILE_FIELD_LIST \
    X(bool, bobber)           \
    X(bool, active)        \
    X(int, owner)                      \
    X(int, identity)                   \
    X(int, type)                       \
    X(int, damage)                     \
    X(int, bannerIdToRespondTo)        \
    X(float, knockBack)                \
    X(int, originalDamage)             \
    X(BNM::Structures::Mono::Array<int>*, ai)

#define INSTANCE_PROJECTILE_METHOD_LIST \
    Z(void, FishingCheck)    \
    Z(void, FishingCheck_RollItemDrop)  \
    Z(void, SetDefaults)

#define STATIC_PROJECTILE_METHOD_LIST \
    T(BNM::UnityEngine::Object*, GetNoneSource)

#define STATIC_PROJECTILE_OVERLOADS(U) \
    U(int, NewProjectile)

class Projectile : public TerrariaBase<Projectile> {
private:
    Projectile() : TerrariaBase(oxorany("Terraria"), oxorany("Projectile")) {
        #define X(type, name) INIT_FIELD(name)
                INSTANCE_PROJECTILE_FIELD_LIST
        #undef X

        #define Z(returnType, name) INIT_METHOD(name)
                INSTANCE_PROJECTILE_METHOD_LIST
        #undef Z

        #define T(returnType, name) INIT_METHOD(name)
                STATIC_PROJECTILE_METHOD_LIST
        #undef T
        NewProjectile_m = _class.GetMethod(oxorany("NewProjectile"), {oxorany("spawnSource"), oxorany("position"), oxorany("velocity"), oxorany("Type"), oxorany("Damage"), oxorany("KnockBack"), oxorany("Owner"), oxorany("ai0"), oxorany("ai1"), oxorany("ai2"), oxorany("modifer")});
    }
    friend class TerrariaBase<Projectile>;

public:
    #define X(type, name) DECLARE_FIELD(type, name)
        INSTANCE_PROJECTILE_FIELD_LIST
    #undef X

    #define Z(returnType, name) DECLARE_METHOD(returnType, name)
        INSTANCE_PROJECTILE_METHOD_LIST
    #undef Z


    #define T(returnType, name) DECLARE_METHOD(returnType, name)
        STATIC_PROJECTILE_METHOD_LIST
    #undef T

    #define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
        INSTANCE_PROJECTILE_FIELD_LIST
    #undef X

    #define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
        INSTANCE_PROJECTILE_METHOD_LIST
    #undef Z

    #define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
        STATIC_PROJECTILE_METHOD_LIST
    #undef T

    #define U(returnType, name) DECLARE_METHOD(returnType, name)
        STATIC_PROJECTILE_OVERLOADS(U)
    #undef U


    #define U(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
        STATIC_PROJECTILE_OVERLOADS(U)
    #undef U

    static int SpawnProjectileSync(int type, int damage, float konback, BNM::Structures::Unity::Vector2 pos, BNM::Structures::Unity::Vector2 vel, int ai0, int ai1, int ai2){
        auto index = 0;
        auto projs = Main::getprojectileSync()->ToVector();
        BNM::UnityEngine::Object *current = nullptr;
        for (int i = 0; i < projs.size(); ++i) {
            auto proj = projs[i];
            if(!getactiveSync(proj)){
                index = i;
                current = proj;
                break;
            }
        }
        auto ai = BNM::Structures::Mono::Array<int>::Create(3);
        ai->CopyFrom({ai0, ai1, ai2});
        SetDefaults_SyncCall(current, type);
        settypeSync(current, type);
        setactiveSync(current, true);
        setdamageSync(current, damage);
        setidentitySync(current, type);
        setaiSync(current, ai);
        Entity::setpositionSync(current, pos);
        Entity::setvelocitySync(current, vel);
        setownerSync(current, Main::getmyPlayerSync());
        return index;
    }
};
