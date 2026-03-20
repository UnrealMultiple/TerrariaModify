#pragma once

#include "TerrariaSupport/TerrariaBase.hpp"

#define STATIC_ITEMID_FIELD_LIST \
    X(short, Count)

class ItemID : public TerrariaBase<ItemID> {
private:
    ItemID() : TerrariaBase(oxorany("Terraria.ID"), oxorany("ItemID")) {
        ctor = _class.GetMethod(".cctor");
#define X(type, name) INIT_FIELD(name)
        STATIC_ITEMID_FIELD_LIST
#undef X

    }
    friend class TerrariaBase<ItemID>;

public:
    BNM::Method<void> ctor;
#define X(type, name) DECLARE_FIELD(type, name)
    STATIC_ITEMID_FIELD_LIST
#undef X


#define X(type, name) DEFINE_STATIC_FIELD_ACCESSORS(name, type, name##_f)
    STATIC_ITEMID_FIELD_LIST
#undef X
};