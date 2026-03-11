#pragma once
#include "Player.hpp"

class SelectedItemState{

private:
    SelectedItemState(){
        _class = Player::Instance()._class.GetInnerClass(oxorany("SelectedItemState"));
        Select_m = _class.GetMethod(oxorany("Select"));
        Update_m = _class.GetMethod(oxorany("Update"));
    }

    BNM::Class _class;

    ~SelectedItemState() = default;

public:
    BNM::Method<void> Update_m;
    BNM::Method<void> Select_m;

    static SelectedItemState& Instance() {
        static SelectedItemState instance;
        return instance;
    }

    SelectedItemState(const SelectedItemState&) = delete;
    SelectedItemState& operator=(const SelectedItemState&) = delete;
};