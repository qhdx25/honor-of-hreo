#ifndef BULLET12_H
#define BULLET12_H

#include "bullet.h"

class Bullet12 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet12(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET12_H
