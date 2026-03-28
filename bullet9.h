#ifndef BULLET9_H
#define BULLET9_H

#include "bullet.h"

class Bullet9 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet9(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 20.0,
            qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET9_H
