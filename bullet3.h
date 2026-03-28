#ifndef BULLET3_H
#define BULLET3_H

#include "bullet.h"

class Bullet3 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet3(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = 16.0,
            qreal maxDistance = 1600.0);

    int damage() const override;
};

#endif // BULLET3_H
