#include "bullet11.h"

QSize Bullet11::defaultSize()
{
    return QSize(GameConfig::kBulletWheelBulletWidth, GameConfig::kBulletWheelBulletHeight);
}

Bullet11::Bullet11(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet11.png", speed, maxDistance, defaultSize())
{
}

int Bullet11::damage() const
{
    return GameConfig::kBulletWheelBulletDamage;
}

