#pragma once

#include "TerrariaBase.hpp"

#define STATIC_LIGHT_OVERLOADS \
    U(void, AddLight)

class Lighting : public TerrariaBase<Lighting> {
private:
    Lighting() : TerrariaBase("Terraria", "Lighting") {

        AddLight_m = _class.GetMethod("AddLight", {"position", "rgb"});
    }
    friend class TerrariaBase<Lighting>;

public:

#define U(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_LIGHT_OVERLOADS
#undef U

#define U(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_LIGHT_OVERLOADS
#undef U

};