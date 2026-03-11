#pragma once
#include "TerrariaBase.hpp"
#include <algorithm>

// ==================== 普通静态方法列表 ====================
#define STATIC_NET_MESSAGE_METHODS(T) \
    T(bool, TrySendData) \
    T(void, SendData) \
    T(void, WriteAccessoryVisibility) \
    T(void, CompressTileBlock) \
    T(bool, DoesPlayerSlotCountAsAHost) \
    T(void, CompressTileBlock_Inner) \
    T(void, DecompressTileBlock) \
    T(void, DecompressTileBlock_Inner) \
    T(void, BootPlayer) \
    T(void, SendObjectPlacement) \
    T(bool, SendTemporaryAnimation) \
    T(void, SendPlayerHurt) \
    T(void, SendPlayerDeath) \
    T(void, PlayNetSound) \
    T(bool, SendCoinLossRevengeMarker) \
    T(void, SendTravelShop) \
    T(void, SendAnglerQuest) \
    T(void, SendSection) \
    T(void, greetPlayer) \
    T(bool, sendWater) \
    T(void, SyncDisconnectedPlayer) \
    T(void, SyncConnectedPlayer) \
    T(void, SendNPCHousesAndTravelShop) \
    T(bool, EnsureLocalPlayerIsPresent) \
    T(void, SyncOnePlayer) \
    T(void, SyncOnePlayer_ItemArray)

// ==================== 重载方法列表（使用不同的成员变量名）====================
#define STATIC_NET_MESSAGE_OVERLOADS(U) \
    U(void, SendTileSquare_ByXY) \
    U(void, SendTileSquare_BySize) \
    U(void, SendTileSquare_Simple)

struct PacketData {
    int msgType;
    int remoteClient = -1;
    int ignoreClient = -1;
    BNM::UnityEngine::Object* text = nullptr;
    int number = 0;
    float number2 = 0;
    float number3 = 0;
    float number4 = 0;
    int number5 = 0;
    int number6 = 0;
    int number7 = 0;
};

class NetMessage : public TerrariaBase<NetMessage> {
private:
    NetMessage() : TerrariaBase("Terraria", "NetMessage") {
        // 初始化普通方法
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_NET_MESSAGE_METHODS(T)
#undef T

        // 初始化重载方法 - 直接指定参数列表
        SendTileSquare_ByXY_m = _class.GetMethod("SendTileSquare", {"whoAmi", "tileX", "tileY", "xSize", "ySize", "changeType"});
        SendTileSquare_BySize_m = _class.GetMethod("SendTileSquare", {"whoAmi", "tileX", "tileY", "centeredSquareSize", "changeType"});
        SendTileSquare_Simple_m = _class.GetMethod("SendTileSquare", {"whoAmi", "tileX", "tileY", "changeType"});
    }
    friend class TerrariaBase<NetMessage>;

public:
    // ==================== 声明普通静态方法 ====================
#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_NET_MESSAGE_METHODS(T)
#undef T

    // ==================== 声明重载方法 ====================
#define U(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_NET_MESSAGE_OVERLOADS(U)
#undef U

    // ==================== 生成普通方法的包装器 ====================
#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_NET_MESSAGE_METHODS(T)
#undef T

    // ==================== 生成重载方法的包装器 ====================
#define U(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_NET_MESSAGE_OVERLOADS(U)
#undef U

    // ==================== 工具方法 ====================
    static void SendDataSync(const PacketData& data) {
        SendData_SyncCall(data.msgType, data.remoteClient, data.ignoreClient, data.text,
                          data.number, data.number2, data.number3, data.number4,
                          data.number5, data.number6, data.number7);
    }

    static void SendData(const PacketData& data) {
        EventUpdateHandler::GetInstance().AddEventR(SendDataSync, data);
    }

    // ==================== 友好的重载方法包装 ====================
    static void SendTileSquareByXY(int whoAmi, int tileX, int tileY, int xSize, int ySize, int changeType) {
        SendTileSquare_ByXY_Call(whoAmi, tileX, tileY, xSize, ySize, changeType);
    }

    static void SendTileSquareBySize(int whoAmi, int tileX, int tileY, int centeredSquareSize, int changeType) {
        SendTileSquare_BySize_Call(whoAmi, tileX, tileY, centeredSquareSize, changeType);
    }

    static void SendTileSquare(int whoAmi, int tileX, int tileY, int changeType) {
        SendTileSquare_Simple_Call(whoAmi, tileX, tileY, changeType);
    }
};