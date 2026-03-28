#ifndef BULLET8_H
#define BULLET8_H

#include "bullet.h"

class Bullet8 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet8(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 20.0,
            qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET8_H
