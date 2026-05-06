#ifndef BULLET6_H
#define BULLET6_H

#include "bullet.h"

class Bullet6 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet6(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
            qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET6_H

