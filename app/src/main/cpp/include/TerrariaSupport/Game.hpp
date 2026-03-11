#pragma once
#include "TerrariaBase.hpp"


#define INSTANCE_GAME_PROPERTY_LIST \
    Y(bool, IsActive)

#define INSTANCE_GAME_METHOD_LIST \
    Z(void, Run)

class Game : public TerrariaBase<Game> {
private:
    Game() : TerrariaBase(oxorany("Microsoft.Xna.Framework.Game"), oxorany("Game")) {
#define Y(type, name) INIT_PROPERTY(name)
        INSTANCE_GAME_PROPERTY_LIST
#undef Y

#define Z(returnType, name) INIT_METHOD(name)
        INSTANCE_GAME_METHOD_LIST
#undef Z
    }
    friend class TerrariaBase<Game>;

public:
#define Y(type, name) DECLARE_PROPERTY(type, name)
    INSTANCE_GAME_PROPERTY_LIST
#undef Y

#define Z(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_GAME_METHOD_LIST
#undef Z

#define Y(type, name) DEFINE_INSTANCE_PROPERTY_ACCESSORS(name, type, name##_p)
    INSTANCE_GAME_PROPERTY_LIST
#undef Y

#define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_GAME_METHOD_LIST
#undef Z
};