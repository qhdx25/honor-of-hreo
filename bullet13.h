#ifndef BULLET13_H
#define BULLET13_H

#include "bullet.h"

class Bullet13 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet13(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
             qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET13_H

