#include "bullet16.h"

QSize Bullet16::defaultSize()
{
    return QSize(120, 34);
}

Bullet16::Bullet16(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet16.png", speed, maxDistance, defaultSize())
{
}

int Bullet16::damage() const
{
    return 30;
}
