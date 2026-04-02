#pragma once

#include "TerrariaBase.hpp"

#define STATIC_WORLDGEN_PROPERTY \
    X(bool, LanternsUp)

#define STATIC_WORLDGEN_METHODS(T) \
    T(void, ToggleManualLanterns)


class LanternNight : public TerrariaBase<LanternNight> {
private:
    LanternNight() : TerrariaBase(oxorany("Terraria.GameContent.Events"), oxorany("LanternNight")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_WORLDGEN_METHODS(T)
#undef T

#define X(type, name) INIT_PROPERTY(name)
        STATIC_WORLDGEN_PROPERTY
#undef X
    }
    friend class TerrariaBase<LanternNight>;

public:
#define X(type, name) DECLARE_PROPERTY(type, name)
    STATIC_WORLDGEN_PROPERTY
#undef X

#define X(type, name) DEFINE_STATIC_PROPERTY_ACCESSORS(name, type, name##_p)
    STATIC_WORLDGEN_PROPERTY
#undef X

#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T
};