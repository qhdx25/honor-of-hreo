#include "bullet8.h"

QSize Bullet8::defaultSize()
{
    return QSize(120, 34);
}

Bullet8::Bullet8(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos, targetPos, "bullet8.png", speed, maxDistance, defaultSize())
{
}

int Bullet8::damage() const
{
    return 30;
}
