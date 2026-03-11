#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_TILEDRAW_METHODS(T) \
    T(void, DrawSingleTile_Flames)


class TileDrawing : public TerrariaBase<TileDrawing> {
private:
    TileDrawing() : TerrariaBase("Terraria.GameContent.Drawing", "TileDrawing") {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        INSTANCE_TILEDRAW_METHODS(T)
#undef T
    }
    friend class TerrariaBase<TileDrawing>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILEDRAW_METHODS(T)
#undef T
};