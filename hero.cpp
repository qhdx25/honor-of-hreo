#include "hero.h"
#include "config.h"

#include <algorithm>
#include <cmath>

hero::hero()
{
    setPosition(100, 100);
    Hero_speed = 10;
}

void hero::shoot()
{
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
