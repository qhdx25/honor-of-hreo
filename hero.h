#ifndef HERO_H
#define HERO_H

#include "config.h"

#include <QPointF>

class hero
{
public:
    hero();
    void shoot();//射击
    QPointF shootOrigin() const;//发射位置函数
    void setPosition(int x, int y);
    void updatePos(const QPointF &velocity, int gameWidth, int gameHeight);
    void resetState();
    void takeDamage(int amount);
    void heal(int amount);
    int gainExperience(int amount);
    int hp() const;
    int maxHp() const;
    qreal hpRatio() const;
    int level() const;
    int maxLevel() const;
    int experience() const;
    int experienceToNextLevel() const;
    qreal experienceRatio() const;
    bool isMaxLevel() const;
    int unlockedSkillCount() const;
    int Hero_x;
    int Hero_y;
    int Hero_speed;

private:
    QPointF m_precisePosition;//精确坐标
    int m_maxHp = GameConfig::kHeroMaxHp;
    int m_hp = GameConfig::kHeroMaxHp;
    int m_level = 1;
    int m_experience = 0;
};

#endif // HERO_H
