#include <jni.h>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>
#include "../include/Tools/obfuscate.h"
#include "../include/oxorany/oxorany_include.h"
#include "../include/KittyMemory/KittyInclude.hpp"
#include "../include/KittyMemory/StealthPatch.h"
#include "menu/MenuSerialize.hpp"
#include "menu/FlexibleWeapon.hpp"
#include "Tools/Logger.hpp"
#include "BNM/Loading.hpp"
#include "TerrariaSupport/NPC.hpp"
#include "TerrariaSupport/Player.hpp"
#include "TerrariaSupport/Ulits/Modfity.hpp"
#include "TerrariaSupport/TerrariaBase.hpp"
#include "TerrariaSupport/MessageBuffer.hpp"
#include "TerrariaSupport/SelectedItemState.hpp"
#include "TerrariaSupport/SpriteBatch.hpp"
#include "TerrariaSupport/Spawner.hpp"
#include "TerrariaSupport/TileDrawing.hpp"
#include "TerrariaSupport/ChatHelper.hpp"
#include "Tools/Tools.h"
#include "Dobby/dobby.h"


ElfScanner g_il2cppElf;
JavaVM* g_vm = nullptr;

void main_thread() {
    sleep(2);
    while (!(g_il2cppElf = ElfScanner::findElf("libil2cpp.so")).isValid()) {
        usleep(100000);
    }
    uintptr_t il2cppBase = g_il2cppElf.base();
    StealthPatch::createWithHex("libil2cpp.so", 0x100000, "00 00 00 00").Modify();
    MemoryPatch::createWithHex(il2cppBase + 0x100000, "00 00 00 00").Modify();
}

MenuOption buildMenu() {
    return MenuOption{
        .pages{
            PageOption{
                .id = 0,
                .title = oxorany("玩家功能"),
                .icon = "icons/func1.png",
                .items = {
                        TitleItem{
                                .label = oxorany("Player")
                        },
                        CheckItem{
                                .label = oxorany("上帝模式"),
                                .id = 100
                        },
                        CheckItem{
                                .label = oxorany("无限召唤"),
                                .id = 101
                        },
                        CheckItem{
                                .label = oxorany("无限范围"),
                                .id = 102,
                        },
                        CheckItem{
                                .label = oxorany("无限魔力"),
                                .id = 103,
                        },
                        CheckItem{
                                .label = oxorany("自动攻击"),
                                .id = 104,
                        },
                        CheckItem{
                                .label = oxorany("司命赐我"),
                                .id = 105,
                        },
                        CheckItem{
                                .label = oxorany("禁止被传送"),
                                .id = 106,
                        },
                        CheckItem{
                                .label = oxorany("解锁全成就"),
                                .id = 107
                        },
                        CheckItem{
                                .label = oxorany("地图高亮"),
                                .id = 108
                        },
                        CheckItem{
                                .label = oxorany("pvp屠杀玩家"),
                                .id = 117
                        },
                        TitleItem{
                                .label = oxorany("GHOST")
                        },
                        CheckItem{
                                .label = oxorany("幽灵模式"),
                                .id = 109
                        },
                        SliderItem{
                                .label = oxorany("移动速度"),
                                .id = 110,
                                .min = 1,
                                .max = 100,
                                .defalutValue = 25
                        },
                        TitleItem{
                                .label = oxorany("TILE")
                        },
                        CheckItem{
                                .label = oxorany("范围挖掘"),
                                .id = 111,
                        },
                        CheckItem{
                                .label = oxorany("破坏墙体"),
                                .id = 112,
                        },
                        SliderItem{
                                .label = oxorany("破坏半径"),
                                .id = 113,
                                .min = 1,
                                .max = 100,
                                .defalutValue = 40
                        },
                        TitleItem{
                                .label = oxorany("SPAWN")
                        },
                        CheckItem{
                                .label = oxorany("启用重生"),
                                .id = 114,
                        },
                        CheckItem{
                                .label = oxorany("拦截重生包"),
                                .id = 115,
                        },
                        SliderItem{
                                .label = oxorany("重生时间"),
                                .id = 116,
                                .min = 1,
                                .max = 30,
                                .defalutValue = 15
                        }
                }
            },
            PageOption{
                .id = 1,
                .title = oxorany("世界功能"),
                .icon = "icons/func2.png",
                .items = {
                    TitleItem{
                        .label = oxorany("World")
                    },
                    CheckItem{
                        .label = oxorany("点亮地图"),
                        .id = 200
                    },
                    CheckItem{
                        .label = oxorany("世界传送"),
                        .id = 201
                    },
                    TitleItem{
                      .label = oxorany("Time")
                    },
                    InputTextItem{
                        .label = oxorany("设置时间"),
                        .id = 202,
                        .placeholder = oxorany("24小时格式"),
                        .default_value = oxorany("6:30")
                    },
                    TitleItem{
                        .label = oxorany("World Event")
                    },
                    SpinnerItem{
                        .label = oxorany("世界事件"),
                        .id = 203,
                        .options = {oxorany("哥布林入侵"), oxorany("雪人入侵"), oxorany("海盗入侵"),  oxorany("火星人"), oxorany("南瓜月"), oxorany("霜月"), oxorany("陨石"), oxorany("满月"), oxorany("血月"), oxorany("日食"), oxorany("沙尘暴"), oxorany("雨天"), oxorany("流星雨"), oxorany("大风天"), oxorany("史莱姆雨"), oxorany("硬币雨"), oxorany("灯笼夜") }
                    },
                    ButtonItem{
                        .label = oxorany("生成事件"),
                        .id = 204,
                        .action = oxorany("default")
                    },
                    ButtonItem{
                            .label = oxorany("停止事件"),
                            .id = 205,
                            .action = oxorany("default")
                    }
                }
            },
            PageOption{
                .id = 2,
                .title = oxorany("NPC功能"),
                .icon = "icons/func3.png",
                .items = {
                    TitleItem{
                            .label = oxorany("KILL")
                    },
                    CheckItem{
                            .label = oxorany("击杀敌方NPC"),
                            .id = 300
                    },
                    CheckItem{
                            .label = oxorany("击杀所有NPC"),
                            .id = 301
                    },
                    CheckItem{
                            .label = oxorany("自动击杀敌对NPC"),
                            .id = 302
                    },
                    TitleItem{
                            .label = oxorany("Spawn")
                    },
                    SliderItem{
                            .label = oxorany("生成延迟"),
                            .id = 303,
                            .min = 1,
                            .max = 1000,
                            .defalutValue = 600
                    },
                    SliderItem{
                            .label = oxorany("生成阈值"),
                            .id = 304,
                            .min = 0,
                            .max = 200,
                            .defalutValue = 5
                    },
                    CheckItem{
                            .label = oxorany("保持修改"),
                            .id = 305
                    },


                }
            },
            PageOption{
                .id = 3,
                .title = oxorany("自动钓鱼"),
                .icon = "icons/fish.png",
                .items = {
                    TitleItem{
                        .label = oxorany("FISH")
                    },
                    CheckItem{
                        .label = oxorany("启用钓鱼"),
                        .id = 400,
                    },
                    CheckItem{
                            .label = oxorany("接受物品"),
                            .id = 401,
                    },
                    CheckItem{
                            .label = oxorany("接受npc"),
                            .id = 402,
                    },
                    CheckItem{
                            .label = oxorany("接受宝匣"),
                            .id = 403,
                    },
                    CheckItem{
                            .label = oxorany("接受普通"),
                            .id = 404,
                    },
                    CheckItem{
                            .label = oxorany("接受常见"),
                            .id = 405,
                    },
                    CheckItem{
                            .label = oxorany("接受罕见"),
                            .id = 406,
                    },
                    CheckItem{
                            .label = oxorany("接受稀有"),
                            .id = 407,
                    },
                    CheckItem{
                            .label = oxorany("接受神话"),
                            .id = 408,
                    },
                    CheckItem{
                            .label = oxorany("接受传奇"),
                            .id = 409,
                    },
                    CheckItem{
                            .label = oxorany("接受任务鱼"),
                            .id = 410,
                    },
                    CheckItem{
                            .label = oxorany("使用当前光标位置"),
                            .id = 411,
                    }
                }
            },
            PageOption{
                .id = 4,
                .title = oxorany("工具"),
                .icon = "icons/tools.png",
                .items = {
                    TitleItem{
                            .label = oxorany("TOOLS")
                        },
                    ButtonItem{
                            .label = oxorany("物品生成器"),
                            .action = oxorany("item_generator_dialog"),
                            .color = static_cast<int>(0xFF708090)
                        },
                    ButtonItem{
                            .label = oxorany("物品修改器"),
                            .action = oxorany("item_modifier_dialog"),
                            .color = static_cast<int>(0xFFFF8C00)
                        },
                    ButtonItem{
                            .label = oxorany("buff生成器"),
                            .action = oxorany("buff_generator_dialog"),
                            .color = static_cast<int>(0xFFAFEEEE)
                    },
                    ButtonItem{
                            .label = oxorany("Boss生成"),
                            .action = oxorany("npc_generator_dialog"),
                            .color = static_cast<int>(0xFFCD5C5C)
                    },
                    TitleItem{
                            .label = oxorany("APPLY PROJICTILE")
                    },
                    CheckItem{
                        .label = oxorany("附加弹幕"),
                        .id = 500
                    },
                    ButtonItem{
                            .label = oxorany("附加配置"),
                            .action = oxorany("projectile_apply_dialog"),
                            .color = static_cast<int>(0xFFCD5C5C)
                    },
                    TitleItem{
                            .label = oxorany("LUA ENGINE")
                        },
                    ButtonItem{
                            .label = oxorany("Lua脚本"),
                            .action = oxorany("lua_script_dialog"),
                            .color = static_cast<int>(0xFF10B981)
                    }
                }
            }
        }
    };
}


__attribute__((constructor)) void init() {
    std::thread(main_thread).detach();
}

std::string GetStringUTF(JNIEnv* env, jstring jst) {
    if (jst == nullptr) {
        return "";
    }
    const char* chars = env->GetStringUTFChars(jst, nullptr);
    if (chars == nullptr) {
        return "";
    }
    std::string str(chars);
    env->ReleaseStringUTFChars(jst, chars);

    return str;
}

void OnLoaded() {
    BNM::BasicHook(Main::Instance().Update_m, TerrariaMainUpdate,old_MainUpdate);
    BNM::BasicHook(Main::Instance().UpdateWorldPreparationState_m, UpdateWorldPreparationState_HOOK,old_UpdateWorldPreparationState_m);
    BNM::BasicHook(Main::Instance().Initialize_m, MainInitializeHOOK, old_MainInitialize);
    BNM::BasicHook(Main::Instance().DrawBlack_m, DrawBlackHOOK, old_DrawBlack);
    BNM::BasicHook(Main::Instance().TriggerPing_m, TriggerPingHook, old_TriggerPing);
    BNM::BasicHook(Player::Instance().ItemCheckWrapped_m, PlayerCheckItemHook,old_PlayerCheckItem);
    BNM::BasicHook(Player::Instance().ResetEffects_m, PlayerResetEffectsHook,old_PlayerResetEffects);
    BNM::BasicHook(Player::Instance().RecalculateLuck_m, RecalculateLuck_HOOK,old_RecalculateLuck_m);
    BNM::BasicHook(Player::Instance().Update_m, PlayerUpdateHook, old_PlayerUpdate);
    BNM::BasicHook(Player::Instance().SlopeDownMovement_m, SlopeDownMovementHOOK, old_SlopeDownMovement);
    BNM::BasicHook(Player::Instance().ItemCheck_StartActualUse_m, ItemCheck_StartActualUseHOOK, old_ItemCheck_StartActualUse);
    BNM::BasicHook(Player::Instance().GetRespawnTime_m, PlayerGetRespawnTimeHOOK, old_PlayerGetRespawnTime);
    BNM::BasicHook(Player::Instance().Teleport_m, PlayerTeleportHOOK, old_PlayerTeleport);
    BNM::BasicHook(Player::Instance().TileCollision_m, TileCollisionHOOK, old_TileCollision);
    BNM::BasicHook(Player::Instance().DryCollision_m, DryCollisionHOOK, old_DryCollision);
    BNM::BasicHook(Player::Instance().SlopingCollision_m, SlopingCollisionHOOK, old_SlopingCollision);
    BNM::BasicHook(Player::Instance().WetCollision_m, WetCollisionHOOK, old_WetCollision);
    BNM::BasicHook(Player::Instance().AddBuff_m, AddBuffHOOK, old_AddBuff);
    BNM::BasicHook(Player::Instance().Hurt_m, PlayerHurtHOOK, old_PLayerHurt);
    BNM::BasicHook(Player::Instance().KillMe_m, PlayerKillMeHOOK, old_PlayerKillMe);
    BNM::BasicHook(Player::Instance().ItemCheck_UseMiningTools_ActuallyUseMiningTool_m, ItemCheck_UseMiningTools_ActuallyUseMiningToolHOOK, old_ItemCheck_UseMiningTools_ActuallyUseMiningTool_m);
    BNM::BasicHook(SpriteBatch::Instance().Draw_Fast_Color_m, BatchDrawFastColorHOOK, old_BatchDrawFastColor);
    BNM::BasicHook(Collision::Instance().StepUp_m, SetUpHOOK, old_SetUp);
    BNM::BasicHook(Collision::Instance().StepDown_m, SetDownHOOK, old_SetDown);
    BNM::BasicHook(Projectile::Instance().FishingCheck_RollItemDrop_m,FishingCheck_RollItemDropHook, old_FishingCheck_RollItemDrop);
    BNM::BasicHook(Spawner::Instance().GetSpawnRate_m, GetSpawnRate_HOOK, old_GetSpawnRate_m);
    BNM::BasicHook(MessageBuffer::Instance().TrySendingItemArray_m, TrySendingItemArray_HOOK, old_TrySendingItemArray_m);
    BNM::BasicHook(MessageBuffer::Instance().ProcessData_m, ProcessDataHOOK, old_ProcessData);
    BNM::BasicHook(SelectedItemState::Instance().Select_m, SelectedItemStateSelect_HOOK, old_SelectedItemStateSelect);
    BNM::BasicHook(SelectedItemState::Instance().Update_m, SelectedItemStateUpdate_HOOK, old_SelectedItemStateUpdate);
    BNM::BasicHook(ChatHelper::Instance().DisplayMessage_m, DisplayMessage, old_DisplayMessage);
    BNM::BasicHook(NetMessage::Instance().DecompressTileBlock_Inner_m,  DecompressTileBlock_InnerHook, old_DecompressTileBlock_Inner);
    BNM::BasicHook(Recipe::Instance().SetupRecipes_m, SetupRecipesHOOK, old_SetupRecipes);
    BNM::BasicHook(ItemID::Instance().ctor, ItemIDCtorHook, old_ItemIDCtor);
}

extern "C"{
    JNIEXPORT jstring JNICALL
    Java_zig_cheat_qq_jni_Jni_getFeatures(JNIEnv* env, jclass clazz) {
        MenuOption cfg = buildMenu();
        nlohmann::json j = cfg;
        std::string jsonStr = j.dump();
        return env->NewStringUTF(jsonStr.c_str());
    }

    JNIEXPORT void JNICALL
    Java_zig_cheat_qq_jni_Jni_Callback(JNIEnv* env, jclass clazz, jint id, jboolean check, jint value, jfloat value2, jstring value3) {
        auto& state =  UIState::getPanelState<UIState::PanelState>();
        auto& playerState = UIState::getPanelState<UIState::PlayerState>();
        auto& worldState = UIState::getPanelState<UIState::WolldState>();
        auto& npcState = UIState::getPanelState<UIState::NPCState>();
        auto& fishState = UIState::getPanelState<UIState::FishUIState>();
        switch (id) {
            case 100: playerState.GodMode = check; break;
            case 101: playerState.InfiniteMinions = check; break;
            case 102: playerState.InfiniteReach = check; break;
            case 103: playerState.InfiniteMana = check; break;
            case 104: playerState.AutoAim = check; break;
            case 105: playerState.bestow = check; break;
            case 106: playerState.AllowTeleport = check; break;
            case 107: Main::UnLockAchievements(); break;
            case 108: FullLigth(check); break;
            case 109: playerState.ghost = check; break;
            case 110: playerState.ghostSpeed = value; break;
            case 111: playerState.KillTileRect = check; break;
            case 112: playerState.KillWallRect = check; break;
            case 113: playerState.DestructionRange = value; break;
            case 114: playerState.respawn = check; break;
            case 115: playerState.interceptRespawnPack = check; break;
            case 116: playerState.respawnSecond = value; break;
            case 117: playerState.AutoButcherPlayer = check; break;
            case 200: if(check) Main::LigthMap(); break;
            case 201: worldState.MapTeleport = check; break;
            case 202: ProcessTimeStringRobust(GetStringUTF(env, value3)); break;
            case 203: worldState.WorldInvasion = static_cast<Invasion>(value + 1); break;
            case 204: StartInvasion(worldState.WorldInvasion); break;
            case 205: StopInvasion(); break;
            case 300: NPC::ButcherAllHostileNPCs(); break;
            case 301: NPC::ButcherAllNPCs(); break;
            case 302: npcState.AutoButcherNPC = check; break;
            case 303: npcState.defaultSpawnRate = value; break;
            case 304: npcState.defaultMaxSpawns = value; break;
            case 305: npcState.modifySpawn = check; break;
            case 400: fishState.AutoFish = check; break;
            case 401: fishState.AutoFish_Item = check; break;
            case 402: fishState.AutoFish_NPC = check; break;
            case 403: fishState.AutoFish_Crates = check; break;
            case 404: fishState.AutoFish_Normal = check; break;
            case 405: fishState.AutoFish_Common = check; break;
            case 406: fishState.AutoFish_Uncommon = check; break;
            case 407: fishState.AutoFish_Rare = check; break;
            case 408: fishState.AutoFish_VeryRare = check; break;
            case 409: fishState.AutoFish_Legendary = check; break;
            case 410: fishState.AutoFish_QuestFish = check; break;
            case 411: fishState.HasSpecialPosition = check; break;
            case 500: state.applyProjectile = check; break;
        }
    }

    JNIEXPORT jstring JNICALL
    Java_zig_cheat_qq_jni_Jni_getCurrentItemJson(JNIEnv *env, jclass clazz) {
        auto& itemState = UIState::getPanelState<UIState::ItemState>();
        auto weapon = FlexibleWeapon(
            Item::GetName(itemState.type),
            itemState.type,
            {
                {"前缀", itemState.prefix},
                {"堆叠", itemState.stack},
                {"伤害", itemState.damage},
                {"大小", itemState.scale},
                {"击退", itemState.knockBack},
                {"弹幕", itemState.shoot},
                {"使用帧", itemState.useTime},
                {"使用动画", itemState.useAnimation},
                {"创建墙", itemState.createWall},
                {"创建图格", itemState.createTile},
                {"创建样式", itemState.placeStyle},
                {"回复生命", itemState.healLife},
                {"回复魔力", itemState.healMana},
                {"颜色R", itemState.colorR},
                {"颜色G", itemState.colorG},
                {"颜色B", itemState.colorB},
                {"颜色A", itemState.colorA}
            }
        );
        return env->NewStringUTF(weapon.dump(4).c_str());
    }

    JNIEXPORT jboolean JNICALL
    Java_zig_cheat_qq_jni_Jni_updateItemAttributes(JNIEnv *env, jclass clazz, jstring json) {
        auto str = GetStringUTF(env, json);
        auto weapon = FlexibleWeapon(str);

        bool anyChanged = false;
        auto& itemState = UIState::getPanelState<UIState::ItemState>();

        if (weapon.attributes.contains("前缀")) {
            int newValue = weapon.attributes["前缀"].get<int>();
            if (itemState.prefix != newValue) {
                itemState.prefix = newValue;
                Item::setprefix(itemState.selectItem, (std::byte)newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("堆叠")) {
            int newValue = weapon.attributes["堆叠"].get<int>();
            if (itemState.stack != newValue) {
                itemState.stack = newValue;
                Item::setstack(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("伤害")) {
            int newValue = weapon.attributes["伤害"].get<int>();
            if (itemState.damage != newValue) {
                itemState.damage = newValue;
                Item::setdamage(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("大小")) {
            int newValue = weapon.attributes["大小"].get<int>();
            if (itemState.scale != newValue) {
                itemState.scale = newValue;
                Item::setscale(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("击退")) {
            int newValue = weapon.attributes["击退"].get<int>();
            if (itemState.knockBack != newValue) {
                itemState.knockBack = newValue;
                Item::setknockBack(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("弹幕")) {
            int newValue = weapon.attributes["弹幕"].get<int>();
            if (itemState.shoot != newValue) {
                itemState.shoot = newValue;
                Item::setshoot(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("使用帧")) {
            int newValue = weapon.attributes["使用帧"].get<int>();
            if (itemState.useTime != newValue) {
                itemState.useTime = newValue;
                Item::setuseTime(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("使用动画")) {
            int newValue = weapon.attributes["使用动画"].get<int>();
            if (itemState.useAnimation != newValue) {
                itemState.useAnimation = newValue;
                Item::setuseAnimation(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("创建墙")) {
            int newValue = weapon.attributes["创建墙"].get<int>();
            if (itemState.createWall != newValue) {
                itemState.createWall = newValue;
                Item::setcreateWall(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("创建图格")) {
            int newValue = weapon.attributes["创建图格"].get<int>();
            if (itemState.createTile != newValue) {
                itemState.createTile = newValue;
                Item::setcreateTile(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("创建样式")) {
            int newValue = weapon.attributes["创建样式"].get<int>();
            if (itemState.placeStyle != newValue) {
                itemState.placeStyle = newValue;
                Item::setplaceStyle(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("回复生命")) {
            int newValue = weapon.attributes["回复生命"].get<int>();
            if (itemState.healLife != newValue) {
                itemState.healLife = newValue;
                Item::sethealLife(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        if (weapon.attributes.contains("回复魔力")) {
            int newValue = weapon.attributes["回复魔力"].get<int>();
            if (itemState.healMana != newValue) {
                itemState.healMana = newValue;
                Item::sethealMana(itemState.selectItem, newValue);
                anyChanged = true;
            }
        }

        bool colorChanged = false;

        if (weapon.attributes.contains("颜色R")) {
            int newValue = weapon.attributes["颜色R"].get<int>();
            if (itemState.colorR != newValue) {
                itemState.colorR = newValue;
                colorChanged = true;
            }
        }

        if (weapon.attributes.contains("颜色G")) {
            int newValue = weapon.attributes["颜色G"].get<int>();
            if (itemState.colorG != newValue) {
                itemState.colorG = newValue;
                colorChanged = true;
            }
        }

        if (weapon.attributes.contains("颜色B")) {
            int newValue = weapon.attributes["颜色B"].get<int>();
            if (itemState.colorB != newValue) {
                itemState.colorB = newValue;
                colorChanged = true;
            }
        }

        if (weapon.attributes.contains("颜色A")) {
            int newValue = weapon.attributes["颜色A"].get<int>();
            if (itemState.colorA != newValue) {
                itemState.colorA = newValue;
                colorChanged = true;
            }
        }

        if (colorChanged) {
            Item::setcolor(itemState.selectItem,
                           BNM::Structures::Unity::Color(
                                   (float)itemState.colorR,
                                   (float)itemState.colorG,
                                   (float)itemState.colorB,
                                   (float)itemState.colorA
                           ));
            anyChanged = true;
        }

        return anyChanged ? JNI_TRUE : JNI_FALSE;
    }

    JNIEXPORT void JNICALL
    Java_zig_cheat_qq_jni_Jni_generatorItem(JNIEnv *env, jclass clazz, jint type, jint num, jstring name) {
        auto player = Main::getLocalPlayer();
        auto pos = Player::GetPosition();
        auto w = Entity::getwidth(player);
        auto h = Entity::getheight(player);
        Item::NewItem((int)pos.x, (int)pos.y, w, h, type, num);
    }

    JNIEXPORT void JNICALL
    Java_zig_cheat_qq_jni_Jni_generatorBuff(JNIEnv *env, jclass clazz, jint type, jint time,
                                            jstring name) {
        Player::AddBUff(type, time * 60);
    }

    JNIEXPORT void JNICALL
    Java_zig_cheat_qq_jni_Jni_generatorNPC(JNIEnv *env, jclass clazz, jint type, jint num,
                                           jstring name) {
        for (int i = 0; i < num; ++i) {
            Spawn(type);
        }
    }

    JNIEXPORT jboolean JNICALL
    Java_zig_cheat_qq_jni_Jni_shouldShowMenu(JNIEnv *env, jclass clazz) {
        auto& state = UIState::getPanelState<UIState::PanelState>();
        return !state.GameMenu;
    }

    JNIEXPORT void JNICALL
    Java_zig_cheat_qq_jni_Jni_attachBulletConfig(JNIEnv *env, jclass clazz, jstring json_str) {
        if(!json_str) return;
        auto& state = UIState::getPanelState<UIState::PanelState>();
        auto str = GetStringUTF(env, json_str);
        nlohmann::json bulletJson = nlohmann::json::parse(str);
        state.ProjectileConfig = bulletJson.get<bullet_config::BulletConfig>();
    }

    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void * reserved)
    {
        g_vm = vm;
        JNIEnv *env;
        vm->GetEnv((void **) &env, JNI_VERSION_1_6);
        BNM::Loading::AllowLateInitHook();
        BNM::Loading::AddOnLoadedEvent(OnLoaded);
        BNM::Loading::TryLoadByJNI(env);
        Tools::InitJniHelper(env);
        return JNI_VERSION_1_6;
    }
}