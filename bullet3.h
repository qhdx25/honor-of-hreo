#ifndef BULLET3_H
#define BULLET3_H

#include "bullet.h"

class Bullet3 : public Bullet
{
public:
    static QSize defaultSize();

    Bullet3(const QPointF &startPos,
            const QPointF &targetPos,
            qreal speed = GameConfig::kCrystalBulletSpeed,
            qreal maxDistance = GameConfig::kCrystalBulletDistance);

    int damage() const override;
};

#endif // BULLET3_H

