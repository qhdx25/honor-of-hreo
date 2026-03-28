#include "bullet10.h"

QSize Bullet10::defaultSize()
{
    return QSize(120, 34);
}

Bullet10::Bullet10(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet10.png", speed, maxDistance, defaultSize())
{
}

int Bullet10::damage() const
{
    return 30;
}
