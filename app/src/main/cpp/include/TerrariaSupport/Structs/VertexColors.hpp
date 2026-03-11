#pragma

#include "Color.hpp"

struct VertexColors {
    Color TopLeftColor;
    Color TopRightColor;
    Color BottomLeftColor;
    Color BottomRightColor;

    VertexColors(Color color)
            : VertexColors(color, color, color, color) {}

    VertexColors(Color tl, Color tr, Color bl, Color br)
            : TopLeftColor(tl)
            , TopRightColor(tr)
            , BottomLeftColor(bl)
            , BottomRightColor(br) {}
};