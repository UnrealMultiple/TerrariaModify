#pragma once
#include "TerrariaBase.hpp"

#define INSTANCE_LANGUAGE_MANAGER_METHOD_LIST \
    Z(void, SetLanguage1) \
    Z(void, SetLanguage2) \
    Z(void, SetLanguage3)

#define STATIC_LANGUAGE_MANAGER_FIELD_LIST \
    S(BNM::UnityEngine::Object*, Instance)

class LanguageManager : public TerrariaBase<LanguageManager> {
private:
    LanguageManager() : TerrariaBase("Terraria.Localization", "LanguageManager") {
#define Z(returnType, name) INIT_METHOD(name)
        INSTANCE_LANGUAGE_MANAGER_METHOD_LIST
#undef Z

#define S(type, name) INIT_FIELD(name)
        STATIC_LANGUAGE_MANAGER_FIELD_LIST
#undef S
    }
    friend class TerrariaBase<LanguageManager>;

public:
#define Z(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_LANGUAGE_MANAGER_METHOD_LIST
#undef Z

#define S(type, name) DECLARE_STATIC_FIELD(type, name)
    STATIC_LANGUAGE_MANAGER_FIELD_LIST
#undef S

#define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_LANGUAGE_MANAGER_METHOD_LIST
#undef Z

#define S(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_LANGUAGE_MANAGER_FIELD_LIST
#undef S
};