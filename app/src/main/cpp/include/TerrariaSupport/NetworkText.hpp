#pragma once
#include "TerrariaBase.hpp"

#define STATIC_NETWORK_TEXT_METHOD_LIST \
    T(BNM::UnityEngine::Object*, FromLiteral)

class NetworkText : public TerrariaBase<NetworkText> {
private:
    NetworkText() : TerrariaBase("Terraria.Localization", "NetworkText") {
#define T(returnType, name) INIT_METHOD(name)
        STATIC_NETWORK_TEXT_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<NetworkText>;

public:
#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_NETWORK_TEXT_METHOD_LIST
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_NETWORK_TEXT_METHOD_LIST
#undef T

    static BNM::UnityEngine::Object* FromLiteral(std::string text) {
        auto string = BNM::CreateMonoString(text);
        return FromLiteral_SyncCall(string);
    }
};