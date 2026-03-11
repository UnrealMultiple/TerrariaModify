#pragma once

#include <vector>
#include "MenuItem.hpp"

struct PageOption{
    int id;
    std::string title;
    std::string icon;
    std::vector<MenuItem> items;
};
