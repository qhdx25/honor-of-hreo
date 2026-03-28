#ifndef BULLET13_H
#define BULLET13_H

#include "bullet.h"

class Bullet13 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet13(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET13_H
