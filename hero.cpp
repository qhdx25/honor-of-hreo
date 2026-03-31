#include "hero.h"
#include "config.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace {
constexpr int kHeroMaxLevel = 4;

int experienceRequiredForLevel(int level)
{
    static constexpr std::array<int, kHeroMaxLevel + 1> kExperienceRequirements = {0, 0, 480, 1200, 2160};
    const int clampedLevel = std::clamp(level, 1, kHeroMaxLevel);
    return kExperienceRequirements.at(clampedLevel);
}
}

hero::hero()
{
    setPosition(100, 100);
    Hero_speed = 10;
    resetState();
}

void hero::shoot()
{
}

void hero::resetState()
{
    m_hp = m_maxHp;
    m_level = 1;
    m_experience = 0;
}

void hero::takeDamage(int amount)
{
    if (amount <= 0 || m_hp <= 0) {
        return;
    }

    m_hp -= amount;
    if (m_hp < 0) {
        m_hp = 0;
    }
}

void hero::heal(int amount)
{
    if (amount <= 0 || m_hp >= m_maxHp) {
        return;
    }

    m_hp += amount;
    if (m_hp > m_maxHp) {
        m_hp = m_maxHp;
    }
}

int hero::gainExperience(int amount)
{
    if (amount <= 0 || isMaxLevel()) {
        return 0;
    }

    m_experience += amount;

    int levelsGained = 0;
    while (m_level < kHeroMaxLevel && m_experience >= experienceRequiredForLevel(m_level + 1)) {
        ++m_level;
        ++levelsGained;
    }

    if (m_level >= kHeroMaxLevel) {
        m_level = kHeroMaxLevel;
        m_experience = experienceRequiredForLevel(kHeroMaxLevel);
    }

    return levelsGained;
}

int hero::hp() const
{
    return m_hp;
}

int hero::maxHp() const
{
    return m_maxHp;
}

qreal hero::hpRatio() const
{
    if (m_maxHp <= 0) {
        return 0.0;
    }

    return static_cast<qreal>(m_hp) / static_cast<qreal>(m_maxHp);
}

int hero::level() const
{
    return m_level;
}

int hero::maxLevel() const
{
    return kHeroMaxLevel;
}

int hero::experience() const
{
    return m_experience;
}

int hero::experienceToNextLevel() const
{
    if (isMaxLevel()) {
        return 0;
    }

    return experienceRequiredForLevel(m_level + 1) - m_experience;
}

qreal hero::experienceRatio() const
{
    if (isMaxLevel()) {
        return 1.0;
    }

    const int currentLevelExperience = experienceRequiredForLevel(m_level);
    const int nextLevelExperience = experienceRequiredForLevel(m_level + 1);
    const int segmentExperience = std::max(1, nextLevelExperience - currentLevelExperience);
    const int progressExperience = std::clamp(m_experience - currentLevelExperience, 0, segmentExperience);
    return static_cast<qreal>(progressExperience) / static_cast<qreal>(segmentExperience);
}

bool hero::isMaxLevel() const
{
    return m_level >= kHeroMaxLevel;
}

int hero::unlockedSkillCount() const
{
    return std::max(0, m_level - 1);
}

QPointF hero::shootOrigin() const
{
    return QPointF(Hero_x + 48, Hero_y + 36);
}

void hero::setPosition(int x, int y)
{
    Hero_x = x;
    Hero_y = y;
    m_precisePosition = QPointF(Hero_x, Hero_y);
}

void hero::updatePos(const QPointF &velocity, int gameWidth, int gameHeight)
{
    m_precisePosition += velocity;

    const qreal maxX = std::max(0, gameWidth - HERO_WIDTH);
    const qreal maxY = std::max(0, gameHeight - HERO_HEIGHT);
    m_precisePosition.setX(std::clamp(m_precisePosition.x(), 0.0, maxX * 1.0));
    m_precisePosition.setY(std::clamp(m_precisePosition.y(), 0.0, maxY * 1.0));

    Hero_x = static_cast<int>(std::lround(m_precisePosition.x()));
    Hero_y = static_cast<int>(std::lround(m_precisePosition.y()));
}
