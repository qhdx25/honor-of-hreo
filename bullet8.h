#ifndef BULLET8_H
#define BULLET8_H

#include "bullet.h"

class Bullet8 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet8(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
            qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET8_H

