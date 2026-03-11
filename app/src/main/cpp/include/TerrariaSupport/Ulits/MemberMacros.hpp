// MemberMacros.hpp
#pragma once

#include <utility>      // std::forward
#include <type_traits>  // std::is_void_v (C++17)
#include "EventUpdateHandler.hpp"

// ==================== 字段声明 ====================
#define DECLARE_FIELD(type, name) BNM::Field<type> name##_f;
#define DECLARE_STATIC_FIELD(type, name) BNM::Field<type> name##_f;

// ==================== 属性声明 ====================
#define DECLARE_PROPERTY(type, name) BNM::Property<type> name##_p;
#define DECLARE_STATIC_PROPERTY(type, name) BNM::Property<type> name##_p;

// ==================== 方法声明 ====================
#define DECLARE_METHOD(returnType, name) BNM::Method<returnType> name##_m;
#define DECLARE_STATIC_METHOD(returnType, name) BNM::Method<returnType> name##_m;

// ==================== 字段/属性初始化 ====================
#define INIT_FIELD(name) name##_f = _class.GetField(#name);
#define INIT_PROPERTY(name) name##_p = _class.GetProperty(#name);
#define INIT_METHOD(name) name##_m = _class.GetMethod(#name);

// 重载方法初始化 - 支持任意数量参数
#define INIT_METHOD_OVERLOAD(name, ...) name##_m = _class.GetMethod(#name, {__VA_ARGS__});

// ==================== 实例字段访问器 ====================
#define DEFINE_INSTANCE_FIELD_ACCESSORS(memberName, memberType, memberVariable) \
    static memberType get##memberName(BNM::UnityEngine::Object* obj) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, obj]() -> memberType { \
            return instance.get##memberName##Sync(obj); \
        }); \
        return future.get(); \
    } \
    static memberType get##memberName##Sync(BNM::UnityEngine::Object* obj) { \
        return Instance().memberVariable[obj].Get(); \
    } \
    static void set##memberName(BNM::UnityEngine::Object* obj, memberType value) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, obj, value]() { \
            instance.set##memberName##Sync(obj, value); \
        }); \
        future.get(); \
    } \
    static void set##memberName##Sync(BNM::UnityEngine::Object* obj, memberType value) { \
        Instance().memberVariable[obj].Set(value); \
    }

// ==================== 实例属性访问器 ====================
#define DEFINE_INSTANCE_PROPERTY_ACCESSORS(memberName, memberType, memberVariable) \
    static memberType get##memberName(BNM::UnityEngine::Object* obj) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, obj]() -> memberType { \
            return instance.get##memberName##Sync(obj); \
        }); \
        return future.get(); \
    } \
    static memberType get##memberName##Sync(BNM::UnityEngine::Object* obj) { \
        return Instance().memberVariable[obj].Get(); \
    } \
    static void set##memberName(BNM::UnityEngine::Object* obj, memberType value) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, obj, value]() { \
            instance.set##memberName##Sync(obj, value); \
        }); \
        future.get(); \
    } \
    static void set##memberName##Sync(BNM::UnityEngine::Object* obj, memberType value) { \
        Instance().memberVariable[obj].Set(value); \
    }

// ==================== 静态字段访问器 ====================
#define DEFINE_STATIC_FIELD_ACCESSORS(memberName, memberType, memberVariable) \
    static memberType get##memberName() { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance]() -> memberType { \
            return instance.get##memberName##Sync(); \
        }); \
        return future.get(); \
    } \
    static memberType get##memberName##Sync() { \
        return Instance().memberVariable.Get(); \
    } \
    static void set##memberName(memberType value) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, value]() { \
            instance.set##memberName##Sync(value); \
        }); \
        future.get(); \
    } \
    static void set##memberName##Sync(memberType value) { \
        Instance().memberVariable.Set(value); \
    }

// ==================== 静态属性访问器 ====================
#define DEFINE_STATIC_PROPERTY_ACCESSORS(memberName, memberType, memberVariable) \
    DEFINE_STATIC_FIELD_ACCESSORS(memberName, memberType, memberVariable)

// ==================== 实例方法调用包装 ====================
#define DEFINE_INSTANCE_METHOD_WRAPPER(returnType, methodName) \
    template<typename... Args> \
    static returnType methodName##_Call(BNM::UnityEngine::Object* obj, Args&&... args) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, obj, args...]() -> returnType { \
            return instance.methodName##_SyncCall(obj, args...); \
        }); \
        if constexpr (std::is_void_v<returnType>) { \
            future.get(); \
        } else { \
            return future.get(); \
        } \
    } \
    template<typename... Args> \
    static returnType methodName##_SyncCall(BNM::UnityEngine::Object* obj, Args&&... args) { \
        return Instance().methodName##_m[obj].Call(std::forward<Args>(args)...); \
    }

// ==================== 静态方法调用包装 ====================
#define DEFINE_STATIC_METHOD_WRAPPER(returnType, methodName) \
    template<typename... Args> \
    static returnType methodName##_Call(Args&&... args) { \
        auto& instance = Instance(); \
        auto future = EventUpdateHandler::GetInstance().AddEventR([&instance, args...]() -> returnType { \
            return instance.methodName##_SyncCall(args...); \
        }); \
        if constexpr (std::is_void_v<returnType>) { \
            future.get(); \
        } else { \
            return future.get(); \
        } \
    } \
    template<typename... Args> \
    static returnType methodName##_SyncCall(Args&&... args) { \
        return Instance().methodName##_m.Call(std::forward<Args>(args)...); \
    }

// ==================== 便捷宏：一次性定义所有 ====================
#define DEFINE_ALL_INSTANCE_FIELDS(list) \
    private: \
        list(X, INIT_FIELD) \
    public: \
        list(X, DECLARE_FIELD) \
        list(X, DEFINE_INSTANCE_FIELD_ACCESSORS)

#define DEFINE_ALL_INSTANCE_PROPERTIES(list) \
    private: \
        list(Y, INIT_PROPERTY) \
    public: \
        list(Y, DECLARE_PROPERTY) \
        list(Y, DEFINE_INSTANCE_PROPERTY_ACCESSORS)

#define DEFINE_ALL_INSTANCE_METHODS(list) \
    private: \
        list(Z, INIT_METHOD) \
    public: \
        list(Z, DECLARE_METHOD) \
        list(Z, DEFINE_INSTANCE_METHOD_WRAPPER)

#define DEFINE_ALL_STATIC_FIELDS(list) \
    private: \
        list(S, INIT_FIELD) \
    public: \
        list(S, DECLARE_STATIC_FIELD) \
        list(S, DEFINE_STATIC_FIELD_ACCESSORS)

#define DEFINE_ALL_STATIC_PROPERTIES(list) \
    private: \
        list(P, INIT_PROPERTY) \
    public: \
        list(P, DECLARE_STATIC_PROPERTY) \
        list(P, DEFINE_STATIC_PROPERTY_ACCESSORS)

#define DEFINE_ALL_STATIC_METHODS(list) \
    private: \
        list(T, INIT_METHOD) \
    public: \
        list(T, DECLARE_STATIC_METHOD) \
        list(T, DEFINE_STATIC_METHOD_WRAPPER)