#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_BATCH_METHODS(T)           \
    T(void, Draw_Fast_Color)                \
    T(void, Draw_Fast_VertexColors)         \
    T(void, DrawF)                          \
    T(void, DrawD)

class SpriteBatch : public TerrariaBase<SpriteBatch> {
private:
    SpriteBatch() : TerrariaBase(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("SpriteBatch")) {
        Draw_Fast_Color_m = _class.GetMethod(oxorany("Draw_Fast"), {
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("Texture2D")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework"), oxorany("Vector2")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework"), oxorany("Rectangle")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("Color")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("SpriteEffects")).Build()

        });
        Draw_Fast_VertexColors_m = _class.GetMethod(oxorany("Draw_Fast"), {
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("Texture2D")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework"), oxorany("Vector2")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework"), oxorany("Rectangle")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Terraria.Graphics"), oxorany("VertexColors")).Build(),
            BNM::CompileTimeClassBuilder(oxorany("Microsoft.Xna.Framework.Graphics"), oxorany("SpriteEffects")).Build()
        });
        //Microsoft.Xna.Framework.Rectangle
//        Draw_Fast_Color_m = _class.GetMethod(oxorany("Draw_Fast"), { oxorany("texture"), oxorany("position"), oxorany("srcRect"), oxorany("color"), oxorany("effects") });
//        Draw_Fast_VertexColors_m = _class.GetMethod(oxorany("Draw_Fast"), { oxorany("texture"), oxorany("position"), oxorany("srcRect"), oxorany("color"), oxorany("effects") });
    }
    friend class TerrariaBase<SpriteBatch>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_BATCH_METHODS(T)
#undef T
};