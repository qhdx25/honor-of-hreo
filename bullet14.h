#ifndef BULLET14_H
#define BULLET14_H

#include "bullet.h"

class Bullet14 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet14(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET14_H
