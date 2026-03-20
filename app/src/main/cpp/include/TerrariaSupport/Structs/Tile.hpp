#pragma once

#include <sys/types.h>
#include <__cstddef/byte.h>
#include "BNM/Defaults.hpp"

struct Tile{
    static Tile s_Null;
    int _tileOffset;
    static BNM::Types::ushort CurrentSearchUID;
    static BNM::Types::ushort MaxSearchUID;
    const int Type_Solid = 0;
    const int Type_Halfbrick = 1;
    const int Type_SlopeDownRight = 2;
    const int Type_SlopeDownLeft = 3;
    const int Type_SlopeUpRight = 4;
    const int Type_SlopeUpLeft = 5;
    const int Liquid_Water = 0;
    const int Liquid_Lava = 1;
    const int Liquid_Honey = 2;
    const int Liquid_Shimmer = 3;
    bool IsLoaded;
    BNM::Types::ushort type;
    BNM::Types::ushort wall;
    BNM::Types::byte liquid;
    short sTileHeader;
    BNM::Types::byte bTileHeader;
    BNM::Types::byte bTileHeader2;
    BNM::Types::byte bTileHeader3;
    short frameX;
    short frameY;
    BNM::Types::ushort TileSeachUID;
    int collisionType;

    bool active(){
        return (sTileHeader & 0x20) == 32;
    }

    bool inActive(){
        return (sTileHeader & 0x40) == 64;
    }

    bool halfBrick(){
        return (sTileHeader & 0x400) == 1024;
    }

    BNM::Types::byte slope(){
        return (BNM::Types::byte)((sTileHeader & 0x7000) >> 12);
    }
};