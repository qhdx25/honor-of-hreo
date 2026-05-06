#ifndef CONFIG_H
#define CONFIG_H

#include <QtGlobal>
#include <array>
#include <cstddef>
namespace GameConfig {
//游戏窗口基础配置
inline constexpr int kGameWidth = 2200;
inline constexpr int kGameHeight = 1260;
inline constexpr const char kGameTitle[] = "honor of hero";
//英雄基础属性与成长配置
inline constexpr int kHeroWidth = 180;
inline constexpr int kHeroHeight = 180;
inline constexpr int kHeroMaxHp = 1000;
inline constexpr int kHeroBaseSpeed = 10;
inline constexpr int kHeroMaxLevel = 4;
//每级升级所需累计经验，0仅作占位
inline constexpr std::array<int, kHeroMaxLevel + 1> kHeroExperienceRequirements = {0, 0, 480, 1200, 2160};
//英雄发射普攻/技能时的出手偏移
inline constexpr int kHeroShootOriginOffsetX = 48;
inline constexpr int kHeroShootOriginOffsetY = 36;
//通用数学常量
inline constexpr qreal kPi = 3.14159265358979323846;
inline constexpr qreal kHalfPi = kPi / 2.0;
//英雄基础普攻子弹配置
inline constexpr qreal kBulletDefaultSpeed = 18.0;
inline constexpr qreal kBulletDefaultMaxDistance = 700.0;
inline constexpr int kBulletDefaultDamage = 25;
inline constexpr int kBulletDefaultWidth = 76;
inline constexpr int kBulletDefaultHeight = 76;
//二技能子弹默认配置
inline constexpr qreal kSkill2BulletDefaultSpeed = 18.0;
inline constexpr qreal kSkill2BulletDefaultMaxDistance = 360.0;
//二技能伤害相对基础普攻的倍率
inline constexpr int kSkill2BulletDamageMultiplier = 4;
//敌方远程单位基础子弹配置
inline constexpr qreal kEnemyBulletDefaultSpeed = 14.0;
inline constexpr qreal kEnemyBulletDefaultMaxDistance = 2600.0;
inline constexpr int kEnemyBulletDefaultDamage = 22;
inline constexpr int kEnemyBulletWidth = 120;
inline constexpr int kEnemyBulletHeight = 40;
//Boss3 远程子弹配置
inline constexpr qreal kEnemyBullet2DefaultSpeed = 16.0;
inline constexpr qreal kEnemyBullet2DefaultMaxDistance = 3200.0;
inline constexpr int kEnemyBullet2DefaultDamage = 36;
inline constexpr int kEnemyBullet2Width = 132;
inline constexpr int kEnemyBullet2Height = 44;
//水晶攻击弹道配置
inline constexpr qreal kCrystalBulletSpeed = 14.0;
inline constexpr qreal kCrystalBulletDistance = 1800.0;
inline constexpr int kCrystalBulletDamage = 84;
inline constexpr int kCrystalBulletWidth = 96;
inline constexpr int kCrystalBulletHeight = 96;
//防御塔攻击弹道配置
inline constexpr qreal kTowerBulletSpeed = 16.0;
inline constexpr qreal kTowerBulletDistance = 1500.0;
inline constexpr int kTowerBulletDamage = 56;
inline constexpr int kTowerBulletWidth = 132;
inline constexpr int kTowerBulletHeight = 44;
//弹幕轮盘（Bullet Wheel）子弹通用配置
inline constexpr qreal kBulletWheelBulletDefaultSpeed = 20.0;
inline constexpr qreal kBulletWheelBulletMaxDistance = 1100.0;
inline constexpr int kBulletWheelBulletDamage = 30;
inline constexpr int kBulletWheelBulletWidth = 120;
inline constexpr int kBulletWheelBulletHeight = 34;
//回旋镖技能弹道配置
inline constexpr qreal kBoomerangBulletDefaultSpeed = 20.0;
inline constexpr qreal kBoomerangBulletDefaultMaxDistance = 900.0;
inline constexpr int kBoomerangBulletDamage = 55;
inline constexpr int kBoomerangBulletWidth = 176;
inline constexpr int kBoomerangBulletHeight = 176;
//回旋镖返回时判定到达英雄的距离阈值
inline constexpr qreal kBoomerangReturnArriveDistance = 1.0;
//回旋镖每帧自转角速度
inline constexpr qreal kBoomerangRotationStepDegrees = 30.0;
//火龙卷弹道配置
inline constexpr qreal kDragonTornadoSpeed = 11.5;
inline constexpr qreal kDragonTornadoDistance = 980.0;
inline constexpr int kDragonTornadoDefaultDamage = 52;
inline constexpr int kDragonTornadoWidth = 120;
inline constexpr int kDragonTornadoHeight = 120;
//防御塔本体配置
inline constexpr int kTowerWidth = 170;
inline constexpr int kTowerHeight = 250;
inline constexpr int kTowerMaxHp = 900;
inline constexpr qreal kTowerAttackIntervalMs = 2200.0;
inline constexpr int kTowerHpBarHeight = 10;
//水晶本体配置
inline constexpr int kCrystalWidth = 240;
inline constexpr int kCrystalHeight = 240;
inline constexpr int kCrystalMaxHp = 1800;
inline constexpr qreal kCrystalAttackIntervalMs = 1800.0;
inline constexpr int kCrystalHpBarHeight = 12;
//宠物配置
inline constexpr int kPetWidth = 96;
inline constexpr int kPetHeight = 96;
//宠物围绕英雄旋转的半径
inline constexpr qreal kPetOrbitRadius = 132.0;
//宠物环绕角速度
inline constexpr qreal kPetOrbitAngularSpeed = 0.0038;
inline constexpr qreal kPetAttackIntervalMs = 1000.0;
//宠物初始召唤位置在英雄上的 Y 偏移
inline constexpr qreal kPetSummonOffsetY = -32.0;
//宠物轨道的纵向压缩比例
inline constexpr qreal kPetOrbitVerticalScale = 0.68;
//宠物轨道整体纵向偏移
inline constexpr qreal kPetOrbitVerticalOffset = -26.0;
//宠物子弹配置
inline constexpr qreal kPetBulletSpeed = 20.0;
inline constexpr qreal kPetBulletDistance = 720.0;
inline constexpr int kPetBulletWidth = 56;
inline constexpr int kPetBulletHeight = 56;
//药包配置
inline constexpr int kMedicinePackWidth = 116;
inline constexpr int kMedicinePackHeight = 116;
//药包被吃掉后的重生时间
inline constexpr qreal kMedicineRespawnDelayMs = 120000.0;
inline constexpr int kMedicineHealAmount = 280;
//敌人按类型配置，顺序与 Enemy::Type 枚举一致
inline constexpr std::array<int, 9> kEnemyWidthByType = {92, 104, 124, 104, 120, 132, 240, 240, 240};
inline constexpr std::array<int, 9> kEnemyHeightByType = {92, 104, 124, 104, 120, 132, 240, 240, 240};
inline constexpr std::array<int, 9> kEnemyMaxHpByType = {60, 110, 80, 160, 70, 120, 1960, 1960, 860};
inline constexpr std::array<qreal, 9> kEnemySpeedByType = {4.6, 3.6, 3.2, 2.4, 5.2, 2.9, 2.5, 2.5, 2.1};
//到达该距离后敌人可判定为“进入攻击范围”
inline constexpr std::array<qreal, 9> kEnemyReachRadiusByType = {18.0, 22.0, 20.0, 26.0, 19.0, 720.0, 540.0, 540.0, 860.0};
inline constexpr std::array<int, 9> kEnemyAttackDamageByType = {8, 14, 12, 18, 16, 22, 52, 52, 36};
inline constexpr std::array<qreal, 9> kEnemyAttackIntervalMsByType = {780.0, 950.0, 1100.0, 1250.0, 720.0, 1650.0, 1750.0, 1750.0, 325.0};
//击杀不同敌人提供的经验值
inline constexpr std::array<int, 9> kEnemyExperienceByType = {20, 28, 24, 36, 26, 34, 120, 120, 90};
//击退修正系数，值越小越抗击退
inline constexpr qreal kBossKnockbackMultiplier = 0.4;
inline constexpr qreal kTankKnockbackMultiplier = 0.65;
inline constexpr qreal kRangedEnemyKnockbackMultiplier = 0.8;
//Boss 追击英雄的最大距离
inline constexpr qreal kBossFollowDistance = 540.0;
inline constexpr qreal kBoss3FollowDistance = 780.0;
//一技能配置
inline constexpr qreal kSkill1CooldownMs = 2000.0;
inline constexpr int kSkill1BulletCount = 6;
inline constexpr qreal kSkill1SpreadDegrees = 60.0;
//一技能目标点距离英雄的长度
inline constexpr qreal kSkill1TargetDistance = 300.0;
inline constexpr qreal kSkill1BulletSpeed = 24.0;
inline constexpr qreal kSkill1BulletDistance = 960.0;
inline constexpr int kSkill1BulletWidth = 64;
inline constexpr int kSkill1BulletHeight = 64;
//二技能配置
inline constexpr qreal kSkill2CooldownMs = 5000.0;
inline constexpr qreal kSkill2TargetDistance = 480.0;
inline constexpr qreal kSkill2BulletSpeed = 20.0;
inline constexpr qreal kSkill2BulletDistance = 1200.0;
//二技能爆炸表现持续时间
inline constexpr qreal kSkill2ExplosionDurationMs = 280.0;
inline constexpr qreal kSkill2ExplosionMaxRadius = 110.0;
//二技能实际伤害判定半径
inline constexpr qreal kSkill2DamageRadius = 150.0;
inline constexpr qreal kSkill2KnockbackDistance = 95.0;
//三技能配置
inline constexpr qreal kSkill3DurationMs = 320.0;
inline constexpr qreal kSkill3SweepDegrees = 50.0;
inline constexpr qreal kSkill3Range = 820.0;
// 视觉宽度与实际判定宽度分离，便于调手感
inline constexpr qreal kSkill3VisualWidth = 170.0;
inline constexpr qreal kSkill3HitWidth = 120.0;
inline constexpr int kSkill3Damage = 160;
inline constexpr qreal kSkill3CooldownMs = 16000.0;
//六技能（回旋镖）配置
inline constexpr qreal kSkill6CooldownMs = 2000.0;
inline constexpr qreal kSkill6TargetDistance = 420.0;
//闪现配置
inline constexpr qreal kFlashCooldownMs = 6000.0;
inline constexpr qreal kFlashDistance = 260.0;
inline constexpr qreal kFlashEffectDurationMs = 220.0;
inline constexpr qreal kFlashImpactMaxRadius = 86.0;
//治疗技能配置
inline constexpr qreal kTreatmentCooldownMs = 20000.0;
inline constexpr int kTreatmentHealAmount = 220;
//弹幕轮盘技能表现配置
inline constexpr qreal kBulletWheelBurstDurationMs = 360.0;
inline constexpr qreal kBulletWheelBurstMaxRadius = 170.0;
inline constexpr qreal kBulletWheelBurstRange = 520.0;
inline constexpr int kBulletWheelProjectileCount = 12;
//弹幕轮盘 12 发子弹各自的飞行速度
inline constexpr std::array<qreal, kBulletWheelProjectileCount> kBulletWheelSpeeds = {
    14.0, 15.5, 17.0, 18.5, 20.0, 21.5, 23.0, 24.5, 26.0, 27.5, 29.0, 30.5
};
//英雄移动与血条 UI 配置
inline constexpr qreal kHeroMoveFrameDurationMs = 80.0;
inline constexpr qreal kHeroMoveHoldDurationMs = 140.0;
inline constexpr qreal kHeroMoveAcceleration = 0.22;
inline constexpr qreal kHeroMoveBrake = 0.76;
inline constexpr qreal kHeroMoveStopThreshold = 0.2;
inline constexpr int kHeroMoveFrameWidth = kHeroWidth + 20;
inline constexpr int kHeroMoveFrameHeight = kHeroHeight + 20;
inline constexpr int kHeroHpBarWidth = 340;
inline constexpr int kHeroHpBarHeight = 38;
inline constexpr int kHeroWorldHpBarWidth = 170;
inline constexpr int kHeroWorldHpBarHeight = 12;
inline constexpr int kHeroWorldHpBarInnerPadding = 14;
inline constexpr int kHeroExpBarWidth = 156;
inline constexpr int kHeroWorldHpBarOffsetY = -28;
inline constexpr int kHeroExpBarOffsetY = -42;
//其他全局表现配置
inline constexpr qreal kMapDisplayScale = 0.36;
inline constexpr int kDefeatAnimationFrameIntervalMs = 67;
//英雄语音首次播放与循环播放间隔
inline constexpr qreal kHeroVoiceInitialDelayMs = 10000.0;
inline constexpr qreal kHeroVoiceRepeatDelayMs = 120000.0;
//特殊 Boss / 事件刷新时间
inline constexpr qreal kDragonSpawnDelayMs = 120000.0;
inline constexpr qreal kBoss3SpawnDelayMs = 40000.0;
//龙攻击与死亡特效配置
inline constexpr qreal kDragonAttackWaveDurationMs = 520.0;
inline constexpr qreal kDragonAttackWaveMaxRadius = 140.0;
inline constexpr qreal kDragonDeathBurstDurationMs = 900.0;
inline constexpr qreal kDragonDeathBurstMaxRadius = 320.0;
} // namespace GameConfig
inline constexpr int GAME_WIDTH = GameConfig::kGameWidth;
inline constexpr int GAME_HEIGHT = GameConfig::kGameHeight;
inline constexpr const char *GAME_TITLE = GameConfig::kGameTitle;
inline constexpr int HERO_WIDTH = GameConfig::kHeroWidth;
inline constexpr int HERO_HEIGHT = GameConfig::kHeroHeight;

#endif // CONFIG_H
