#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_TILEDRAW_METHODS(T) \
    T(void, Draw_V2ScaleV2) \
    T(void, Draw_V2ScaleFloat) \
    T(void, Draw_Fast_Color) \
    T(void, Draw_Fast_VertexColors)

class SpriteBatch : public TerrariaBase<SpriteBatch> {
private:
    SpriteBatch() : TerrariaBase("Microsoft.Xna.Framework.Graphics.SpriteBatch", "SpriteBatch") {
        Draw_V2ScaleV2_m = _class.GetMethod("Draw", { "texture", "position", "sourceRectangle", "color", "rotation", "origin", "scale", "effects", "layerDepth" });
        Draw_V2ScaleFloat_m = _class.GetMethod("Draw", { "texture", "position", "sourceRectangle", "color", "rotation", "origin", "scale", "effects", "layerDepth" });
        Draw_Fast_Color_m = _class.GetMethod("Draw_Fast", { "texture", "position", "srcRect", "color", "effects" });
        Draw_Fast_VertexColors_m = _class.GetMethod("Draw_Fast", { "texture", "position", "srcRect", "color", "effects" });
    }
    friend class TerrariaBase<SpriteBatch>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_TILEDRAW_METHODS(T)
#undef T
};