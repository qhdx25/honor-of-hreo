#include "bullet12.h"

QSize Bullet12::defaultSize()
{
    return QSize(120, 34);
}

Bullet12::Bullet12(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet12.png", speed, maxDistance, defaultSize())
{
}

int Bullet12::damage() const
{
    return 30;
}
