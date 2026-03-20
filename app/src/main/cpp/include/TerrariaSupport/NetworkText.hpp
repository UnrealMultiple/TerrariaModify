#pragma once
#include "TerrariaBase.hpp"

#define STATIC_NETWORK_TEXT_METHOD_LIST \
    T(BNM::UnityEngine::Object*, FromLiteral)

#define INSTANCE_NETWORK_TEXT_METHOD_LIST \
    Y(BNM::Structures::Mono::String*, ToString)

class NetworkText : public TerrariaBase<NetworkText> {
private:
    NetworkText() : TerrariaBase(oxorany("Terraria.Localization"), oxorany("NetworkText")) {
#define T(returnType, name) INIT_METHOD(name)
        STATIC_NETWORK_TEXT_METHOD_LIST
#undef T
#define Y(returnType, name) INIT_METHOD(name)
        INSTANCE_NETWORK_TEXT_METHOD_LIST
#undef Y

    }
    friend class TerrariaBase<NetworkText>;

public:
#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_NETWORK_TEXT_METHOD_LIST
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_NETWORK_TEXT_METHOD_LIST
#undef T

#define Y(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_NETWORK_TEXT_METHOD_LIST
#undef Y

#define Y(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_NETWORK_TEXT_METHOD_LIST
#undef Y

    static BNM::UnityEngine::Object* FromLiteral(std::string text) {
        auto string = BNM::CreateMonoString(text);
        return FromLiteral_SyncCall(string);
    }
};