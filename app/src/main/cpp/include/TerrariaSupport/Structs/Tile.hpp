#pragma once

#include <sys/types.h>
#include <__cstddef/byte.h>

struct Tile{
    int _tileOffset;
    bool IsLoaded;
    u_short type;
    short wall;
    std::byte liquid;
    std::byte sTileHeader;
    std::byte bTileHeader;
    std::byte bTileHeader2;
    std::byte bTileHeader3;
    short frameX;
    short frameY;
    short TileSeachUID;
    int collisionType;
};