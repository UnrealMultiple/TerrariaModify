#pragma once

#include "TerrariaBase.hpp"
#include "TerrariaSupport/Structs/Tile.hpp"

#define INSTANCE_TILEDRAW_METHODS \
    T(Tile, get_Item)


class TileData : public TerrariaBase<TileData> {
private:
    TileData() : TerrariaBase(oxorany("Terraria"), oxorany("TileData")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        INSTANCE_TILEDRAW_METHODS
#undef T
    }
    friend class TerrariaBase<TileData>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILEDRAW_METHODS
#undef T

#define T(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_TILEDRAW_METHODS
#undef T
};