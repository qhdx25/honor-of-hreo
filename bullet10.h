#ifndef BULLET10_H
#define BULLET10_H

#include "bullet.h"

class Bullet10 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet10(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
             qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET10_H

