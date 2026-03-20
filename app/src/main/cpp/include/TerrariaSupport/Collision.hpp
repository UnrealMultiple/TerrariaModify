
#pragma once

#include "TerrariaBase.hpp"


#define STATIC_COLLISOION_METHOD_LIST \
    T(void, StepDown)         \
    T(void, StepUp)

class Collision : public TerrariaBase<Collision> {
private:
    Collision() : TerrariaBase(oxorany("Terraria"), oxorany("Collision")) {

#define T(returnType, name) INIT_METHOD(name)
        STATIC_COLLISOION_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<Collision>;

public:

#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_COLLISOION_METHOD_LIST
#undef T


#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_COLLISOION_METHOD_LIST
#undef T
};