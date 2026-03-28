#ifndef BULLET6_H
#define BULLET6_H

#include "bullet.h"

class Bullet6 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet6(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 20.0,
            qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET6_H
