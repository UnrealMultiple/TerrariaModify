#pragma once
#include "BNM/UnityStructures/Vector3.hpp"
#include "BNM/UnityStructures/Vector4.hpp"

struct Matrix{
    float M11;
    float M12;
    float M13;
    float M14;
    float M21;
    float M22;
    float M23;
    float M24;
    float M31;
    float M32;
    float M33;
    float M34;
    float M41;
    float M42;
    float M43;
    float M44;
    BNM::Structures::Unity::Vector3 Backward;
    BNM::Structures::Unity::Vector3 Down;
    BNM::Structures::Unity::Vector3 Forward;
    BNM::Structures::Unity::Vector3 Left;
    BNM::Structures::Unity::Vector3 Right;
    BNM::Structures::Unity::Vector3 Translation;
    BNM::Structures::Unity::Vector3 Up;
    static Matrix identity;

    static BNM::Structures::Unity::Vector2 Transform(BNM::Structures::Unity::Vector2 position, Matrix matrix){
        float x = position.x * matrix.M11 + position.y * matrix.M21 + matrix.M41;
        float y = position.x * matrix.M12 + position.y * matrix.M22 + matrix.M42;
        return BNM::Structures::Unity::Vector2(x, y);
    }

};