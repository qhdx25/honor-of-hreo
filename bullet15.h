#ifndef BULLET15_H
#define BULLET15_H

#include "bullet.h"

class Bullet15 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet15(const QPointF &startPos,
             const QPointF &targetPos,
             qreal speed = 20.0,
             qreal maxDistance = 1100.0);

    int damage() const override;
};

#endif // BULLET15_H
