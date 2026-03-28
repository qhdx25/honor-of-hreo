#include "bullet4.h"

QSize Bullet4::defaultSize()
{
    return QSize(132, 44);
}

Bullet4::Bullet4(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos,
             targetPos,
             "bullet4.png",
             speed,
             maxDistance,
             defaultSize())
{
}

int Bullet4::damage() const
{
    return 56;
}
