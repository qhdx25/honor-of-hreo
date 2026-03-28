#include "bullet3.h"

QSize Bullet3::defaultSize()
{
    return QSize(96, 96);
}

Bullet3::Bullet3(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed,
                 qreal maxDistance)
    : Bullet(startPos,
             targetPos,
             "bullet3.png",
             speed,
             maxDistance,
             defaultSize())
{
}

int Bullet3::damage() const
{
    return 84;
}
