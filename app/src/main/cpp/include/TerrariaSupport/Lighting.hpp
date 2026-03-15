#pragma once

#include "TerrariaBase.hpp"

#define STATIC_LIGHT_OVERLOADS \
    U(void, AddLight)

#define STATIC_LIGHT_PROPERTY \
    T(int, Mode)              \
    T(BNM::UnityEngine::Object*, LegacyEngine)  \
    T(BNM::UnityEngine::Object*, _activeEngine)

#define LIGHT_STATIC_METHOD_LIST \
    S(int, set_Mode)

class Lighting : public TerrariaBase<Lighting> {
private:
    Lighting() : TerrariaBase(oxorany("Terraria"), oxorany("Lighting")) {
#define T(type, name) INIT_PROPERTY(name)
        STATIC_LIGHT_PROPERTY
#undef T
#define S(returnType, name) INIT_METHOD(name)
        LIGHT_STATIC_METHOD_LIST
#undef S
        AddLight_m = _class.GetMethod(oxorany("AddLight"), {oxorany("i"), oxorany("j"), oxorany("r"), oxorany("g"), oxorany("b")});
    }
    friend class TerrariaBase<Lighting>;

public:

#define U(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_LIGHT_OVERLOADS
#undef U

#define U(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_LIGHT_OVERLOADS
#undef U

#define S(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    LIGHT_STATIC_METHOD_LIST
#undef S


#define T(type, name) DECLARE_STATIC_PROPERTY(type, name)
    STATIC_LIGHT_PROPERTY
#undef T

#define T(type, name) DEFINE_STATIC_PROPERTY_ACCESSORS(name, type, name##_p)
    STATIC_LIGHT_PROPERTY
#undef T

#define S(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    LIGHT_STATIC_METHOD_LIST
#undef S
};