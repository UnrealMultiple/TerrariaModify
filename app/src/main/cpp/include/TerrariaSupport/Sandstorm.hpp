#pragma once

#include "TerrariaBase.hpp"
#include "Main.hpp"

#define STATIC_WORLDGEN_FIELD \
    X(bool, Happening)

#define STATIC_WORLDGEN_METHODS(T) \
    T(void, StartSandstorm)              \
    T(void, StopSandstorm)


class Sandstorm : public TerrariaBase<Sandstorm> {
private:
    Sandstorm() : TerrariaBase(oxorany("Terraria.GameContent.Events"), oxorany("Sandstorm")) {
#define T(returnType, name) name##_m = _class.GetMethod(#name);
        STATIC_WORLDGEN_METHODS(T)
#undef T

#define X(type, name) INIT_FIELD(name)
        STATIC_WORLDGEN_FIELD
#undef X
    }
    friend class TerrariaBase<Sandstorm>;

public:
#define X(type, name) DECLARE_FIELD(type, name)
    STATIC_WORLDGEN_FIELD
#undef X

#define X(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_WORLDGEN_FIELD
#undef X

#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_WORLDGEN_METHODS(T)
#undef T
};