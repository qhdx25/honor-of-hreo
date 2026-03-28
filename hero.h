#ifndef HERO_H
#define HERO_H

#include <QPointF>

class hero
{
public:
    hero();

    void shoot();
    QPointF shootOrigin() const;
    void updatePos(int key, int gameWidth, int gameHeight);

    int Hero_x;
    int Hero_y;
    int Hero_speed;
};

#endif // HERO_H
