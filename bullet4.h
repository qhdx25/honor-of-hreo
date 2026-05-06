#ifndef BULLET4_H
#define BULLET4_H

#include "bullet.h"

class Bullet4 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet4(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kTowerBulletSpeed,
            qreal maxDistance = GameConfig::kTowerBulletDistance);

    int damage() const override;

protected:
    bool rotatesToVelocity() const override;
};

#endif // BULLET4_H

