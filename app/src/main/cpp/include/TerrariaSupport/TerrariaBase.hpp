#pragma once

#include "BNM/Class.hpp"
#include "BNM/Field.hpp"
#include "BNM/Method.hpp"
#include "BNM/Property.hpp"
#include "BNM/UnityStructures/Color.hpp"
#include "Ulits/MemberMacros.hpp"
#include "BNM/Defaults.hpp"
#include "BNM/UnityStructures/Vector2.hpp"
#include "BNM/ComplexMonoStructures.hpp"
#include "Tools/Logger.hpp"
#include "oxorany/oxorany.h"
#include <type_traits>

template<typename Derived>
class TerrariaBase {
public:
    TerrariaBase(const char* namespaze, const char* name) : _class(namespaze, name) {}

    static Derived& Instance() {
        static Derived instance;
        return instance;
    }

    TerrariaBase(const TerrariaBase&) = delete;
    TerrariaBase& operator=(const TerrariaBase&) = delete;

    BNM::Class _class;

protected:
    ~TerrariaBase() = default;
};