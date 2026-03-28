#ifndef BULLET10_H
#define BULLET10_H

#include "bullet.h"

class Bullet10 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet10(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET10_H
