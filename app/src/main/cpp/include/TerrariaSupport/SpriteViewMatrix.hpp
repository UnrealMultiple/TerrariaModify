#pragma once

#include "TerrariaBase.hpp"
#include "Structs/Matrix.hpp"
#include "Main.hpp"

#define INSTANCE_SPRITEVIEWMATRIX_PROPERTY_LIST \
    X(Matrix, ZoomMatrix) \
    X(BNM::Structures::Unity::Vector2, Zoom)

class SpriteViewMatrix : public TerrariaBase<SpriteViewMatrix> {
private:
    SpriteViewMatrix() : TerrariaBase(oxorany("Terraria.Graphics"), oxorany("SpriteViewMatrix")) {
#define X(type, name) INIT_PROPERTY(name)
        INSTANCE_SPRITEVIEWMATRIX_PROPERTY_LIST
#undef X
    }
    friend class TerrariaBase<SpriteViewMatrix>;

public:
#define X(type, name) DECLARE_PROPERTY(type, name)
    INSTANCE_SPRITEVIEWMATRIX_PROPERTY_LIST
#undef X  // 这里缺少了 X

#define X(type, name) DEFINE_INSTANCE_PROPERTY_ACCESSORS(name, type, name##_p)
    INSTANCE_SPRITEVIEWMATRIX_PROPERTY_LIST
#undef X
};