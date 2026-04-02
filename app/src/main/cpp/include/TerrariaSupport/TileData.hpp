#pragma once

#include "TerrariaBase.hpp"
#include "TerrariaSupport/Structs/Tile.hpp"

#define DEFINE_STATIC_TILE_METHOD_WRAPPER(returnType, methodName)  \
    template<typename... Args>                                     \
    static int methodName(int x, int y, Args&&... args) {          \
        auto offset = CalculateTileOffset(x, y);                   \
        return Instance().methodName##_m(offset, args...);         \
    }

#define INSTANCE_TILE_DATA_METHODS \
    T(Tile, get_Item)

#define INSTANCE_TILE_DATA_FIELD \
    Y(int, _width)               \
    Y(int, _height)

#define STATIC_TILE_DATA_METHODS                 \
    X(BNM::Types::ushort, GetSearchUID)          \
    X(void, SetSearchUID)                        \
    X(BNM::Types::ushort, GetType)               \
    X(void, SetType)                             \
    X(short, GetSHeader)                         \
    X(void, SetSHeader)                          \
    X(short, GetFrameX)                          \
    X(short, GetFrameY)                          \
    X(void, SetFrameX)                           \
    X(void, SetFrameY)                           \
    X(bool, GetCheckingLiquid)                   \
    X(void, SetCheckingLiquid)                   \
    X(bool, GetSkipLiquid)                       \
    X(void, SetSkipLiquid)                       \
    X(bool, GetTileFramed)                       \
    X(void, SetTileFramed)                       \
    X(BNM::Types::ushort, GetWall)               \
    X(void, SetWall)                             \
    X(BNM::Types::byte, GetLiquid)               \
    X(void, SetLiquid)                           \
    X(BNM::Types::byte, GetBHeader)              \
    X(void, SetBHeader)                          \
    X(BNM::Types::byte, GetBHeader2)             \
    X(void, SetBHeader2)                         \
    X(BNM::Types::byte, GetBHeader3)             \
    X(void, SetBHeader3)



class TileData : public TerrariaBase<TileData> {
private:
    TileData() : TerrariaBase(oxorany("Terraria"), oxorany("TileData")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        INSTANCE_TILE_DATA_METHODS
#undef T

#define X(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_TILE_DATA_METHODS
#undef X

#define Y(type, name) INIT_FIELD(name)
        INSTANCE_TILE_DATA_FIELD
#undef Y
    }
    friend class TerrariaBase<TileData>;

    static int CalculateTileOffset(int x, int y){
        auto w = get_widthSync(Main::gettileSync());
        return w * y + x;
    }

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILE_DATA_METHODS
#undef T

#define T(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_TILE_DATA_METHODS
#undef T

#define X(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_TILE_DATA_METHODS
#undef X

#define X(returnType, name) DEFINE_STATIC_TILE_METHOD_WRAPPER(returnType, name)
    STATIC_TILE_DATA_METHODS
#undef X

#define Y(returnType, name) DECLARE_FIELD(returnType, name)
    INSTANCE_TILE_DATA_FIELD
#undef Y

#define Y(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
    INSTANCE_TILE_DATA_FIELD
#undef Y

    static bool active(int x, int y){
        return (GetSHeader(x, y) & 0x60) == 0x20;
    }

    static bool inActive(int x, int y){
        return ((GetSHeader(x, y) >> 6) & 1) != 0;
    }

    static bool halfBrick(int x, int y){
        return ((GetSHeader(x, y) >> 10) & 1) != 0;
    }

    static BNM::Types::byte slope(int x, int y){
        return (BNM::Types::byte)((GetSHeader(x, y) >> 12) & 0x7);
    }

    static int GetCollisionType(int x, int y){
        if (!active(x, y))
        {
            return 0;
        }
        if (halfBrick(x, y))
        {
            return 2;
        }
        if (slope(x, y) > 0)
        {
            return 2 + slope(x, y);
        }
        auto tileSolid = Main::gettileSolidSync()->ToVector();
        auto tileSolidTop = Main::gettileSolidTopSync()->ToVector();
        auto type =  GetType(x, y);
        if (tileSolid[type] && !tileSolidTop[type])
        {
            return 1;
        }
        return ((GetBHeader(x, y) >> 12) & 7) + 2;;
    }

};