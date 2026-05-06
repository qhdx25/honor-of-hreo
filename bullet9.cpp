#include "bullet9.h"

QSize Bullet9::defaultSize()
{
    return QSize(GameConfig::kBulletWheelBulletWidth, GameConfig::kBulletWheelBulletHeight);
}

Bullet9::Bullet9(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet9.png", speed, maxDistance, defaultSize())
{
}

int Bullet9::damage() const
{
    return GameConfig::kBulletWheelBulletDamage;
}

