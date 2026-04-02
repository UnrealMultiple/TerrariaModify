#pragma once
#include "TerrariaBase.hpp"

#define STATIC_LANG_METHOD_LIST \
    T(BNM::Structures::Mono::String*, GetNPCNameValue) \
    T(BNM::Structures::Mono::String*, GetItemNameValue) \
    T(BNM::Structures::Mono::String*, GetBuffName)

class Lang : public TerrariaBase<Lang> {
private:
    Lang() : TerrariaBase(oxorany("Terraria"), oxorany("Lang")) {
#define T(returnType, name) INIT_METHOD(name)
        STATIC_LANG_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<Lang>;

public:
#define T(returnType, name) DECLARE_METHOD(returnType, name)
    STATIC_LANG_METHOD_LIST
#undef T

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_LANG_METHOD_LIST
#undef T

    static std::string GetNPCName(int type) {
        auto feat = EventUpdateHandler::GetInstance().AddEventR([type]() -> std::string {
            auto name = GetNPCNameValue_SyncCall(type);
            return name->str();
        });
        return feat.get();
    }

    static std::string GetItemName(int type) {
        auto feat = EventUpdateHandler::GetInstance().AddEventR([type]() -> std::string {
            auto name = GetItemNameValue_SyncCall(type);
            return name->str();
        });
        return feat.get();
    }

    static std::string GetBuffName(int type) {
        auto feat = EventUpdateHandler::GetInstance().AddEventR([type]() -> std::string {
            auto name = GetBuffName_SyncCall(type);
            return name->str();
        });
        return feat.get();
    }
};