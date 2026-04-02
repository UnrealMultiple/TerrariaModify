#pragma once
#include "TerrariaBase.hpp"

#define STATIC_LANG_METHOD_LIST \
    T(void, DisplayMessage)

class ChatHelper : public TerrariaBase<ChatHelper> {
private:
    ChatHelper() : TerrariaBase(oxorany("Terraria.Chat"), oxorany("ChatHelper")) {
#define T(returnType, name) INIT_METHOD(name)
        STATIC_LANG_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<ChatHelper>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_LANG_METHOD_LIST
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_LANG_METHOD_LIST
#undef T
};