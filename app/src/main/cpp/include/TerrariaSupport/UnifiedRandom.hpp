#pragma once

#include "TerrariaBase.hpp"
#include "Main.hpp"


class UnifiedRandom : public TerrariaBase<UnifiedRandom> {
private:
    UnifiedRandom() : TerrariaBase("Terraria.Utilities", "UnifiedRandom") {
        Next_m = _class.GetMethod("Next", {"minValue", "maxValue"});
    }
    friend class TerrariaBase<UnifiedRandom>;

public:
    BNM::Method<int> Next_m;

    static UnifiedRandom& Instance() {
        static UnifiedRandom instance;
        return instance;
    }

    static int Next(int minValue, int maxValue){
        auto rand = Main::Instance().rand_f();
        return Instance().Next_m[rand](minValue, maxValue);
    }
};