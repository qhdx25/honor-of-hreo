#ifndef BULLET15_H
#define BULLET15_H
#include "bullet.h"
class Bullet15 : public Bullet
{
public:
    static QSize defaultSize();
    Bullet15(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = GameConfig::kBulletWheelBulletDefaultSpeed,
             qreal maxDistance = GameConfig::kBulletWheelBulletMaxDistance);
    int damage() const override;
};

#endif

