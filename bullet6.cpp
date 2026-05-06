#include "bullet6.h"

QSize Bullet6::defaultSize()
{
    return QSize(GameConfig::kBulletWheelBulletWidth, GameConfig::kBulletWheelBulletHeight);
}

Bullet6::Bullet6(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet6.png", speed, maxDistance, defaultSize())
{
}

int Bullet6::damage() const
{
    return GameConfig::kBulletWheelBulletDamage;
}

