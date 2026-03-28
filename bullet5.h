#ifndef BULLET5_H
#define BULLET5_H

#include "bullet.h"

class Bullet5 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet5(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 20.0,
            qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET5_H
