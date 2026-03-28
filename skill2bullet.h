#ifndef SKILL2BULLET_H
#define SKILL2BULLET_H

#include "bullet.h"

class Skill2Bullet : public Bullet
{
public:
    static QSize defaultSize();

    Skill2Bullet(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed = 18.0,
                 qreal maxDistance = 360.0);

    int damage() const override;
};

#endif // SKILL2BULLET_H
