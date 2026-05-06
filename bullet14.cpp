#include "bullet14.h"

QSize Bullet14::defaultSize()
{
    return QSize(GameConfig::kBulletWheelBulletWidth, GameConfig::kBulletWheelBulletHeight);
}

Bullet14::Bullet14(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet14.png", speed, maxDistance, defaultSize())
{
}

int Bullet14::damage() const
{
    return GameConfig::kBulletWheelBulletDamage;
}

