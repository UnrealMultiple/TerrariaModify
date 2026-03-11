#pragma once

#include <sys/types.h>
#include <__cstddef/byte.h>

struct MapTile{
    u_short Type;
    std::byte Light;
    std::byte _extraData;
    bool IsChanged;
    std::byte Color;
};