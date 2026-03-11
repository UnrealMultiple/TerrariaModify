#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_TILEDRAW_METHODS(T) \
    T(void, Draw_V2ScaleV2) \
    T(void, Draw_V2ScaleFloat) \
    T(void, Draw_Fast_Color) \
    T(void, Draw_Fast_VertexColors)

class SpriteBatch : public TerrariaBase<SpriteBatch> {
private:
    SpriteBatch() : TerrariaBase(oxorany("Microsoft.Xna.Framework.Graphics.SpriteBatch"), oxorany("SpriteBatch")) {
        Draw_V2ScaleV2_m = _class.GetMethod(oxorany("Draw"), { oxorany("texture"), oxorany("position"), oxorany("sourceRectangle"), oxorany("color"), oxorany("rotation"), oxorany("origin"), oxorany("scale"), oxorany("effects"), oxorany("layerDepth") });
        Draw_V2ScaleFloat_m = _class.GetMethod(oxorany("Draw"), { oxorany("texture"), oxorany("position"), oxorany("sourceRectangle"), oxorany("color"), oxorany("rotation"), oxorany("origin"), oxorany("scale"), oxorany("effects"), oxorany("layerDepth") });
        Draw_Fast_Color_m = _class.GetMethod(oxorany("Draw_Fast"), { oxorany("texture"), oxorany("position"), oxorany("srcRect"), oxorany("color"), oxorany("effects") });
        Draw_Fast_VertexColors_m = _class.GetMethod(oxorany("Draw_Fast"), { oxorany("texture"), oxorany("position"), oxorany("srcRect"), oxorany("color"), oxorany("effects") });
    }
    friend class TerrariaBase<SpriteBatch>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILEDRAW_METHODS(T)
#undef T
};