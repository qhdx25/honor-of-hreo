#ifndef ENEMYBULLET2_H
#define ENEMYBULLET2_H

#include "bullet.h"

class EnemyBullet2 : public Bullet
{
public:
    static QSize defaultSize();

    EnemyBullet2(const QPointF &startPos,
                 const QPointF &targetPos,
                 int damage = 36,
                 qreal speed = 16.0,
                 qreal maxDistance = 3200.0);

    int damage() const override;

protected:
    bool rotatesToVelocity() const override;

private:
    int m_damage = 36;
};

#endif // ENEMYBULLET2_H
