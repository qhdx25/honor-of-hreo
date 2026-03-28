#ifndef BULLET7_H
#define BULLET7_H

#include "bullet.h"

class Bullet7 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet7(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 20.0,
            qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET7_H
