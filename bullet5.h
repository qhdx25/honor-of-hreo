#ifndef BULLET5_H
#define BULLET5_H

#include "bullet.h"

class Bullet5 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet5(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
            qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET5_H

