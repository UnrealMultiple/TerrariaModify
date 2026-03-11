#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace bullet_config {

    struct Direction {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct SpawnPosition {
        float x = 0.0f;
        float y = 0.0f;
    };

    struct AI {
        int ai1 = 0;
        int ai2 = 0;
        int ai3 = 0;
    };

    struct Bullet {
        int id = 0;
        int damage = 0;
        float fireSpeed = 0.0f;
        int knockback = 0;
        Direction direction;
        bool useCursorPosition = false;
        SpawnPosition spawnPosition;
        bool usePlayerPosition = false;
        AI ai;
    };

    using BulletConfig = std::unordered_map<int, std::vector<Bullet>>;

}

namespace nlohmann {

    template<>
    struct adl_serializer<bullet_config::Direction> {
        static void from_json(const json& j, bullet_config::Direction& d) {
            j.at("x").get_to(d.x);
            j.at("y").get_to(d.y);
        }
        static void to_json(json& j, const bullet_config::Direction& d) {
            j = json{ {"x", d.x}, {"y", d.y} };
        }
    };

    template<>
    struct adl_serializer<bullet_config::SpawnPosition> {
        static void from_json(const json& j, bullet_config::SpawnPosition& s) {
            j.at("x").get_to(s.x);
            j.at("y").get_to(s.y);
        }
        static void to_json(json& j, const bullet_config::SpawnPosition& s) {
            j = json{ {"x", s.x}, {"y", s.y} };
        }
    };

    template<>
    struct adl_serializer<bullet_config::AI> {
        static void from_json(const json& j, bullet_config::AI& a) {
            j.at("ai1").get_to(a.ai1);
            j.at("ai2").get_to(a.ai2);
            j.at("ai3").get_to(a.ai3);
        }
        static void to_json(json& j, const bullet_config::AI& a) {
            j = json{ {"ai1", a.ai1}, {"ai2", a.ai2}, {"ai3", a.ai3} };
        }
    };

    template<>
    struct adl_serializer<bullet_config::Bullet> {
        static void from_json(const json& j, bullet_config::Bullet& b) {
            j.at("id").get_to(b.id);
            j.at("damage").get_to(b.damage);
            j.at("fireSpeed").get_to(b.fireSpeed);
            j.at("knockback").get_to(b.knockback);
            j.at("direction").get_to(b.direction);
            j.at("useCursorPosition").get_to(b.useCursorPosition);
            j.at("spawnPosition").get_to(b.spawnPosition);
            j.at("usePlayerPosition").get_to(b.usePlayerPosition);
            j.at("ai").get_to(b.ai);
        }
        static void to_json(json& j, const bullet_config::Bullet& b) {
            j = json{
                    {"id", b.id},
                    {"damage", b.damage},
                    {"fireSpeed", b.fireSpeed},
                    {"knockback", b.knockback},
                    {"direction", b.direction},
                    {"useCursorPosition", b.useCursorPosition},
                    {"spawnPosition", b.spawnPosition},
                    {"usePlayerPosition", b.usePlayerPosition},
                    {"ai", b.ai}
            };
        }
    };

    template<>
    struct adl_serializer<bullet_config::BulletConfig> {
        static void from_json(const json& j, bullet_config::BulletConfig& config) {
            config.clear();
            for (auto it = j.begin(); it != j.end(); ++it) {
                int weaponId = std::stoi(it.key());
                config[weaponId] = it.value().get<std::vector<bullet_config::Bullet>>();
            }
        }

        static void to_json(json& j, const bullet_config::BulletConfig& config) {
            j = json::object();
            for (const auto& [weaponId, bullets] : config) {
                j[std::to_string(weaponId)] = bullets;
            }
        }
    };

}