#ifndef BULLET11_H
#define BULLET11_H

#include "bullet.h"

class Bullet11 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet11(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET11_H
