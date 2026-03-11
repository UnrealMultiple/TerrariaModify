#pragma once
#include <algorithm>
#include "BNM/UnityStructures/Vector3.hpp"

struct Color{
    std::byte A;
    std::byte B;
    std::byte G;
    std::byte R;
    uint32_t PackedValue;

    Color(BNM::Structures::Unity::Vector3 color){
        PackedValue = 0;
        R = static_cast<std::byte>(std::clamp((int)color.x * 255, 0, 255));
        G = static_cast<std::byte>(std::clamp((int)color.y * 255, 0, 255));
        B = static_cast<std::byte>(std::clamp((int)color.z * 255, 0, 255));
        A = static_cast<std::byte>(255);
    }
};