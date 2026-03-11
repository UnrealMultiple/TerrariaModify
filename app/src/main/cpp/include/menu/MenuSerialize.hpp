#pragma once

#include "MenuItem.hpp"
#include "PageOption.hpp"
#include "MenuOption.hpp"
#include "../../include/nlohmann/json.hpp"

namespace nlohmann {

    // ---------- 原有类型的特化 ----------
    template <>
    struct adl_serializer<TitleItem> {
        static void to_json(json& j, const TitleItem& item) {
            j = json::object();
            j["type"] = "TITLE";
            j["label"] = item.label;
        }
    };

    template <>
    struct adl_serializer<CheckItem> {
        static void to_json(json& j, const CheckItem& item) {
            j = json::object();
            j["type"] = "CHECK";
            j["label"] = item.label;
            j["id"] = item.id;
        }
    };

    template <>
    struct adl_serializer<SliderItem> {
        static void to_json(json& j, const SliderItem& item) {
            j = json::object();
            j["type"] = "SLIDER";
            j["label"] = item.label;
            j["id"] = item.id;
            j["min"] = item.min;
            j["max"] = item.max;
            j["defaultValue"] = item.defalutValue;
        }
    };

    template <>
    struct adl_serializer<SpinnerItem> {
        static void to_json(json& j, const SpinnerItem& item) {
            j = json::object();
            j["type"] = "SPINNER";
            j["label"] = item.label;
            j["id"] = item.id;
            j["options"] = item.options;
        }
    };

    template <>
    struct adl_serializer<ButtonItem> {
        static void to_json(json& j, const ButtonItem& item) {
            j = json::object();
            j["type"] = "BUTTON";
            j["label"] = item.label;
            j["id"] = item.id;
            j["action"] = item.action;
            j["color"] = item.color;
        }
    };

    // ---------- 新增输入类型的特化 ----------
    template <>
    struct adl_serializer<InputTextItem> {
        static void to_json(json& j, const InputTextItem& item) {
            j = json::object();
            j["type"] = "INPUT_TEXT";
            j["label"] = item.label;
            j["id"] = item.id;
            j["placeholder"] = item.placeholder;
            j["default_value"] = item.default_value;
        }
    };

    template <>
    struct adl_serializer<InputIntItem> {
        static void to_json(json& j, const InputIntItem& item) {
            j = json::object();
            j["type"] = "INPUT_INT";
            j["label"] = item.label;
            j["id"] = item.id;
            j["min"] = item.min;
            j["max"] = item.max;
            j["default_value"] = item.default_value;
        }
    };

    template <>
    struct adl_serializer<InputFloatItem> {
        static void to_json(json& j, const InputFloatItem& item) {
            j = json::object();
            j["type"] = "INPUT_FLOAT";
            j["label"] = item.label;
            j["id"] = item.id;
            j["min"] = item.min;
            j["max"] = item.max;
            j["default_value"] = item.default_value;
        }
    };

    template <>
    struct adl_serializer<InputDoubleItem> {
        static void to_json(json& j, const InputDoubleItem& item) {
            j = json::object();
            j["type"] = "INPUT_DOUBLE";
            j["label"] = item.label;
            j["id"] = item.id;
            j["min"] = item.min;
            j["max"] = item.max;
            j["default_value"] = item.default_value;
        }
    };

    // ---------- MenuItem (variant) 的序列化（显式调用） ----------
    template <>
    struct adl_serializer<MenuItem> {
        static void to_json(json& j, const MenuItem& item) {
            std::visit([&j](const auto& concrete) {
                j = concrete;   // 调用具体类型的 to_json
            }, item);
        }
    };

    // ---------- PageOption 和 MenuOption 的序列化（保持不变） ----------
    template <>
    struct adl_serializer<PageOption> {
        static void to_json(json& j, const PageOption& page) {
            j = json::object();
            j["id"] = page.id;
            j["title"] = page.title;
            j["icon"] = page.icon;
            j["items"] = page.items;   // 会使用 MenuItem 的序列化
        }
    };

    template <>
    struct adl_serializer<MenuOption> {
        static void to_json(json& j, const MenuOption& config) {
            j = json::object();
            j["pages"] = config.pages;
        }
    };

} // namespace nlohmann
