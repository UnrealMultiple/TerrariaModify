#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_XNA_UNITY_RUNNER_FIELD                                         \
    T(BNM::Structures::Unity::Vector2, IncrementalBatchingOffset)               \
    T(bool, IncrementalBatching)

#define STATIC_XNA_UNITY_RUNNER_FIELD \
    Y(BNM::UnityEngine::Object*, _instance)


class XNAUnityRunner : public TerrariaBase<XNAUnityRunner> {
private:
    XNAUnityRunner() : TerrariaBase(oxorany(""), oxorany("XNAUnityRunner")) {
#define T(type, name) INIT_FIELD(name)
        INSTANCE_XNA_UNITY_RUNNER_FIELD
#undef T

#define Y(type, name) INIT_FIELD(name)
        STATIC_XNA_UNITY_RUNNER_FIELD
#undef Y
    }
    friend class TerrariaBase<XNAUnityRunner>;

public:
#define T(type, name) DECLARE_FIELD(type, name)
    INSTANCE_XNA_UNITY_RUNNER_FIELD
#undef T

#define T(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
    INSTANCE_XNA_UNITY_RUNNER_FIELD
#undef T


#define Y(type, name) DECLARE_FIELD(type, name)
    STATIC_XNA_UNITY_RUNNER_FIELD
#undef Y

#define Y(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_XNA_UNITY_RUNNER_FIELD
#undef Y

};