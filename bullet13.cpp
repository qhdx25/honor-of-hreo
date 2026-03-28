#include "bullet13.h"

QSize Bullet13::defaultSize()
{
    return QSize(120, 34);
}

Bullet13::Bullet13(const QPointF &startPos,
                   const QPointF &targetPos,
                   qreal speed,
                   qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet13.png", speed, maxDistance, defaultSize())
{
}

int Bullet13::damage() const
{
    return 30;
}
