#pragma once

#include "TerrariaBase.hpp"

#define INSTANCE_ENTITY_FIELD_LIST \
    X(int, whoAmI)           \
    X(BNM::Structures::Unity::Vector2, position) \
    X(BNM::Structures::Unity::Vector2, velocity) \
    X(BNM::Structures::Unity::Vector2,oldPosition)                               \
    X(int, width)            \
    X(int, height)

#define INSTANCE_ENTITY_PROPERTY_LIST \
    Y(BNM::Structures::Unity::Vector2, Center)

class Entity : public TerrariaBase<Entity> {
private:
    Entity() : TerrariaBase(oxorany("Terraria"), oxorany("Entity")) {
#define X(type, name) INIT_FIELD(name)
        INSTANCE_ENTITY_FIELD_LIST
#undef X

#define Y(type, name) INIT_PROPERTY(name)
        INSTANCE_ENTITY_PROPERTY_LIST
#undef Y
    }
    friend class TerrariaBase<Entity>;

public:
#define X(type, name) DECLARE_FIELD(type, name)
    INSTANCE_ENTITY_FIELD_LIST
#undef X

#define Y(type, name) DECLARE_PROPERTY(type, name)
    INSTANCE_ENTITY_PROPERTY_LIST
#undef Y

#define X(type, name) DEFINE_INSTANCE_FIELD_ACCESSORS(name, type, name##_f)
    INSTANCE_ENTITY_FIELD_LIST
#undef X

#define Y(type, name) DEFINE_INSTANCE_PROPERTY_ACCESSORS(name, type, name##_p)
    INSTANCE_ENTITY_PROPERTY_LIST
#undef Y
};