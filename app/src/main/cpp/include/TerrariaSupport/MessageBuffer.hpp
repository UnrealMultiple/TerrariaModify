#pragma once
#include "TerrariaBase.hpp"

#define INSTANCE_MESSAGE_BUFFER_METHOD_LIST \
    Z(void, TrySendingItemArray)            \
    Z(void, ProcessData)

class MessageBuffer : public TerrariaBase<MessageBuffer> {
private:
    MessageBuffer() : TerrariaBase(oxorany("Terraria"), oxorany("MessageBuffer")) {
#define Z(returnType, name) INIT_METHOD(name)
        INSTANCE_MESSAGE_BUFFER_METHOD_LIST
#undef Z
    }
    friend class TerrariaBase<MessageBuffer>;

public:
#define Z(returnType, name) DECLARE_METHOD(returnType, name)
    INSTANCE_MESSAGE_BUFFER_METHOD_LIST
#undef Z

#define Z(returnType, name) DEFINE_INSTANCE_METHOD_WRAPPER(returnType, name)
    INSTANCE_MESSAGE_BUFFER_METHOD_LIST
#undef Z
};