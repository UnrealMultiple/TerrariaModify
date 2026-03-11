#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 使用 struct 定义 FlexibleWeapon
struct FlexibleWeapon {
    std::string name;
    int id;
    json attributes;  // 直接存储为 json 对象

    // 默认构造函数
    FlexibleWeapon() = default;

    FlexibleWeapon(const std::string& n, int i, const json& attrs = {})
            : name(n), id(i), attributes(attrs) {}

    // 从 JSON 构造
    explicit FlexibleWeapon(const json& j) {
        fromJson(j);
    }

    // 从 JSON 字符串构造
    explicit FlexibleWeapon(const std::string& jsonStr) {
        fromJson(json::parse(jsonStr));
    }

    // 从 JSON 对象初始化
    void fromJson(const json& j) {
        if (j.contains("name")) name = j["name"];
        if (j.contains("id")) id = j["id"];
        if (j.contains("attributes")) attributes = j["attributes"];
    }

    // 添加任意属性
    template<typename T>
    void addAttribute(const std::string& key, T value) {
        attributes[key] = value;
    }

    // 获取属性（自动类型推导）
    template<typename T>
    T getAttribute(const std::string& key) const {
        if (!hasAttribute(key)) {
            throw std::runtime_error("Attribute not found: " + key);
        }
        return attributes[key].get<T>();
    }

    // 安全获取属性（带默认值）
    template<typename T>
    T getAttributeOrDefault(const std::string& key, T defaultValue) const {
        if (hasAttribute(key)) {
            return attributes[key].get<T>();
        }
        return defaultValue;
    }

    // 检查属性是否存在
    bool hasAttribute(const std::string& key) const {
        return attributes.contains(key);
    }

    // 移除属性
    void removeAttribute(const std::string& key) {
        attributes.erase(key);
    }

    // 获取所有属性名
    std::vector<std::string> getAttributeNames() const {
        std::vector<std::string> names;
        for (auto& [key, _] : attributes.items()) {
            names.push_back(key);
        }
        return names;
    }

    // 获取属性数量
    size_t getAttributeCount() const {
        return attributes.size();
    }

    // 清空所有属性
    void clearAttributes() {
        attributes.clear();
    }

    // 序列化到 JSON 对象
    json toJson() const {
        json j;
        j["name"] = name;
        j["id"] = id;
        j["attributes"] = attributes;
        return j;
    }

    // 序列化到 JSON 字符串
    std::string dump(int indent = -1) const {
        return toJson().dump(indent);
    }

    // 批量添加属性
    void addAttributes(const json& attrs) {
        for (auto& [key, value] : attrs.items()) {
            attributes[key] = value;
        }
    }

    // 从初始化列表添加属性
    void addAttributes(const std::initializer_list<std::pair<std::string, json>>& attrs) {
        for (const auto& [key, value] : attrs) {
            attributes[key] = value;
        }
    }

    // 运算符重载，方便直接访问属性
    json& operator[](const std::string& key) {
        return attributes[key];
    }

    const json& operator[](const std::string& key) const {
        return attributes[key];
    }
};