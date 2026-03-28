#ifndef HERO_H
#define HERO_H

#include <QPointF>

class hero
{
public:
    hero();

    void shoot();
    QPointF shootOrigin() const;
    void setPosition(int x, int y);
    void updatePos(const QPointF &velocity, int gameWidth, int gameHeight);
    void takeDamage(int amount);
    void heal(int amount);
    int hp() const;
    int maxHp() const;
    qreal hpRatio() const;

    int Hero_x;
    int Hero_y;
    int Hero_speed;

private:
    QPointF m_precisePosition;
    int m_maxHp = 1000;
    int m_hp = 1000;
};

#endif // HERO_H
