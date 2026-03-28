#include "bullet5.h"

QSize Bullet5::defaultSize()
{
    return QSize(120, 34);
}

Bullet5::Bullet5(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet5.png", speed, maxDistance, defaultSize())
{
}

int Bullet5::damage() const
{
    return 30;
}
