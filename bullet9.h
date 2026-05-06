#ifndef BULLET9_H
#define BULLET9_H

#include "bullet.h"

class Bullet9 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet9(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
            qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET9_H

