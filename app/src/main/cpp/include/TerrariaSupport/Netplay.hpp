#pragma once
#include "TerrariaBase.hpp"

#define STATIC_NETPLAY_METHOD_LIST \
    T(void, StartTcpClient)

class Netplay : public TerrariaBase<Netplay> {
private:
    Netplay() : TerrariaBase(oxorany("Terraria"), oxorany("Netplay")) {
#define T(returnType, name) INIT_METHOD(name)
        STATIC_NETPLAY_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<Netplay>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_NETPLAY_METHOD_LIST
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_NETPLAY_METHOD_LIST
#undef T
};