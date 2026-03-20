#pragma once

#include "TerrariaBase.hpp"
#include "Projectile.hpp"
#include "Entity.hpp"
#include "Main.hpp"
#include "NPCSpawnParams.hpp"
#include "NetMessage.hpp"

#define INSTANCE_RECIPE_FIELD_LIST \
    X(BNM::UnityEngine::Object*, createItem)          \
    X(BNM::Structures::Mono::Array<BNM::UnityEngine::Object*>*, requiredItem) \
    X(int, requiredTile)

#define STATIC_RECIPE_STATIC_FIELD \
    Y(BNM::UnityEngine::Object*, currentRecipe) \
    Y(int, numRecipes)

#define INSTANCE_RECIPE_METHOD_LIST \
    Z(void, SetCraftingStation)

#define STATIC_RECIPE_METHOD_LIST \
    T(void, SetupRecipes)         \
    T(void, AddRecipe)            \
    T(void, CreateRequiredItemQuickLookups) \
    T(void, UpdateWhichItemsAreMaterials)

class Recipe : public TerrariaBase<Recipe> {
private:
    Recipe() : TerrariaBase(oxorany("Terraria"), oxorany("Recipe")) {
#define X(type, name) INIT_FIELD(name)
        INSTANCE_RECIPE_FIELD_LIST
#undef X

#define Y(type, name) INIT_FIELD(name)
        STATIC_RECIPE_STATIC_FIELD
#define Y

#define Z(returnType, name) INIT_METHOD(name)
        INSTANCE_RECIPE_METHOD_LIST
#undef Z

#define T(returnType, name) INIT_METHOD(name)
        STATIC_RECIPE_METHOD_LIST
#undef T
    }
    friend class TerrariaBase<Recipe>;

public:
#define X(type, name) DECLARE_FIELD(type, name)
    INSTANCE_RECIPE_FIELD_LIST
#undef X

#define Y(type, name) DECLARE_FIELD(type, name)
    STATIC_RECIPE_STATIC_FIELD
#undef Y

#define Z(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_RECIPE_METHOD_LIST
#undef Z

#define T(returnType, name) DECLARE_STATIC_METHOD(returnType, name)
    STATIC_RECIPE_METHOD_LIST
#undef T

#define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
    INSTANCE_RECIPE_FIELD_LIST
#undef X

#define Y(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_RECIPE_STATIC_FIELD
#undef Y

#define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_RECIPE_METHOD_LIST
#undef Z

#define T(returnType, name) DEFINE_STATIC_METHOD_WRAPPER(returnType, name)
    STATIC_RECIPE_METHOD_LIST
#undef T
};