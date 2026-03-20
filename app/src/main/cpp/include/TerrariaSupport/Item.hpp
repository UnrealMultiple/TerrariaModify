#pragma once
#include "TerrariaBase.hpp"
#include "Main.hpp"
#include "NetMessage.hpp"
#include <algorithm>

#define INSTANCE_ITEM_FIELDS(X) \
    X(int, type) \
    X(int, damage) \
    X(float, knockBack) \
    X(float, scale) \
    X(BNM::Structures::Unity::Color, color) \
    X(std::byte, prefix) \
    X(int, useTime) \
    X(int, shoot) \
    X(int, shootSpeed) \
    X(int, stack) \
    X(int, createTile) \
    X(int, createWall) \
    X(int, healLife) \
    X(int, healMana) \
    X(int, useAnimation) \
    X(int, placeStyle) \
    X(int, maxStack) \
    X(int, fishingPole)

#define INSTANCE_ITEM_PROPERTIES(Y) \
    Y(BNM::Structures::Mono::String*, Name)

#define INSTANCE_ITEM_METHODS(Z) \
    Z(void, netDefaults)         \
    Z(void, SetDefaults)

#define STATIC_ITEM_METHODS(T) \
    T(int, NewItem)

struct ItemResult {
    int type;
    int stack;
    int index;

    ItemResult(int t, int s, int i) : type(t), stack(s), index(i) {}
};

class Item : public TerrariaBase<Item> {
private:
    Item() : TerrariaBase(oxorany("Terraria"), oxorany("Item")) {

#define X(type, name) name##_f = _class.GetField(#name);
        INSTANCE_ITEM_FIELDS(X)
#undef X

#define Y(type, name) name##_p = _class.GetProperty(#name);
        INSTANCE_ITEM_PROPERTIES(Y)
#undef Y

#define Z(returnType, name) name##_m = _class.GetMethod(#name);
        INSTANCE_ITEM_METHODS(Z)
#undef Z
        NewItem_m = _class.GetMethod(oxorany("NewItem"), {oxorany("source"), oxorany("X"), oxorany("Y"), oxorany("Width"), oxorany("Height"), oxorany("Type"), oxorany("Stack"), oxorany("noBroadcast"), oxorany("pfix"), oxorany("noGrabDelay")});
    }
    friend class TerrariaBase<Item>;

public:
#define X(type, name) DECLARE_FIELD(type, name)
    INSTANCE_ITEM_FIELDS(X)
#undef X

#define Y(type, name) DECLARE_PROPERTY(type, name)
    INSTANCE_ITEM_PROPERTIES(Y)
#undef Y

#define Z(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_ITEM_METHODS(Z)
#undef Z

#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_ITEM_METHODS(T)
#undef T

#define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
    INSTANCE_ITEM_FIELDS(X)
#undef X

#define Y(type, name) DEFINE_INSTANCE_PROPERTY_ACCESSORS(name, type, name##_p)
    INSTANCE_ITEM_PROPERTIES(Y)
#undef Y

#define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_ITEM_METHODS(Z)
#undef Z

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_ITEM_METHODS(T)
#undef T

    static std::string GetNameSync(int type) {
        auto item = Instance()._class.CreateNewObjectParameters();
        Instance().netDefaults_m[item].Call(type);
        auto name = Instance().Name_p[item].Get();
        return name ? name->str() : "";
    }

    static std::string GetName(int type) {
        auto feat = EventUpdateHandler::GetInstance().AddEventR(GetNameSync, type);
        return feat.get();
    }

    static ItemResult NewItemSync(int X, int Y, int width, int height, int type, int stack = 1, bool noBroadcast = false, int prefix = 0, bool noGrabDelay = false){
        auto source = Projectile::GetNoneSource_SyncCall();
        auto item = Instance()._class.CreateNewObjectParameters();
        Instance().netDefaults_m[item].Call(type);
        auto maxStack = Instance().maxStack_f[item].Get();
        auto currentStack = std::min(stack, maxStack);
        auto index = Instance().NewItem_m.Call(source, X, Y, width, height, type, currentStack, noBroadcast, prefix, noBroadcast);
        if(Main::getnetModeSync() == 1){
            PacketData packet {
                    .msgType = 21,
                    .number = index,
                    .number2 = 1
            };
            NetMessage::SendDataSync(packet);
        }
        return ItemResult(type, currentStack, index);
    }

    static ItemResult NewItem(int X, int Y, int width, int height, int type, int stack = 1, bool noBroadcast = false, int prefix = 0, bool noGrabDelay = false){
        auto f = EventUpdateHandler::GetInstance().AddEventR(NewItemSync, X, Y, width, height, type, stack, noBroadcast, prefix, noGrabDelay);
        return f.get();
    }
};