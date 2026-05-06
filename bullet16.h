#ifndef BULLET16_H
#define BULLET16_H

#include "bullet.h"

class Bullet16 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet16(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
             qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET16_H

