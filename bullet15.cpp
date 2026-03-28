#include "bullet15.h"

QSize Bullet15::defaultSize()
{
    return QSize(120, 34);
}

Bullet15::Bullet15(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet15.png", speed, maxDistance, defaultSize())
{
}

int Bullet15::damage() const
{
    return 30;
}
