#include "bullet9.h"

QSize Bullet9::defaultSize()
{
    return QSize(120, 34);
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
    return 30;
}
