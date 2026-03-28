#include "bullet7.h"

QSize Bullet7::defaultSize()
{
    return QSize(120, 34);
}

Bullet7::Bullet7(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet7.png", speed, maxDistance, defaultSize())
{
}

int Bullet7::damage() const
{
    return 30;
}
