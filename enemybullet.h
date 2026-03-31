#ifndef ENEMYBULLET_H
#define ENEMYBULLET_H

#include "bullet.h"

class EnemyBullet : public Bullet
{
public:
    static QSize defaultSize();

    EnemyBullet(const QPointF &startPos,
                const QPointF &targetPos,
                int damage = 22,
                qreal speed = 14.0,
                qreal maxDistance = 2600.0);

    int damage() const override;

protected:
    bool rotatesToVelocity() const override;

private:
    int m_damage = 22;
};

#endif // ENEMYBULLET_H
