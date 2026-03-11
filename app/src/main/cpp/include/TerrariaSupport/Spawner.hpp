#pragma once

#include "NPC.hpp"

class Spawner{

private:
    Spawner(){
        _class = NPC::Instance()._class.GetInnerClass("Spawner");
        GetSpawnRate_m = _class.GetMethod("GetSpawnRate");
    }

    BNM::Class _class;

    ~Spawner() = default;

public:
    BNM::Method<void> GetSpawnRate_m;

    static Spawner& Instance() {
        static Spawner instance;
        return instance;
    }

    Spawner(const Spawner&) = delete;
    Spawner& operator=(const Spawner&) = delete;
};