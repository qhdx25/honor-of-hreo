#ifndef BULLET11_H
#define BULLET11_H

#include "bullet.h"

class Bullet11 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet11(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
             qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);

    int damage() const override;
};

#endif // BULLET11_H

