#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_TILEDRAW_METHODS \
    T(void, DrawSingleTile_Flames)   \
    T(void, GetScreenDrawArea)


class TileDrawing : public TerrariaBase<TileDrawing> {
private:
    TileDrawing() : TerrariaBase(oxorany("Terraria.GameContent.Drawing"), oxorany("TileDrawing")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        INSTANCE_TILEDRAW_METHODS
#undef T
    }
    friend class TerrariaBase<TileDrawing>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILEDRAW_METHODS
#undef T

#define T(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_TILEDRAW_METHODS
#undef T
};