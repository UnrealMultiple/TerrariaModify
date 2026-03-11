#pragma once

#include <string>
#include <vector>
#include <variant>

// 原有类型
struct TitleItem {
    std::string label;
};

struct CheckItem {
    std::string label;
    int id;
};

struct SliderItem {
    std::string label;
    int id;
    int min;
    int max;
    int defalutValue;
};

struct SpinnerItem {
    std::string label;
    int id;
    std::vector<std::string> options;
};

struct ButtonItem {
    std::string label;
    int id = -1;
    std::string action;
    int color = 0xFF696969;
};

// 新增输入类型
struct InputTextItem {
    std::string label;
    int id;
    std::string placeholder;
    std::string default_value;
};

struct InputIntItem {
    std::string label;
    int id;
    int min;
    int max;
    int default_value;
};

struct InputFloatItem {
    std::string label;
    int id;
    float min;
    float max;
    float default_value;
};

struct InputDoubleItem {
    std::string label;
    int id;
    double min;
    double max;
    double default_value;
};

// 菜单项变体类型（包含所有新类型）
using MenuItem = std::variant<
        TitleItem,
        CheckItem,
        SliderItem,
        SpinnerItem,
        ButtonItem,
        InputTextItem,
        InputIntItem,
        InputFloatItem,
        InputDoubleItem
>;