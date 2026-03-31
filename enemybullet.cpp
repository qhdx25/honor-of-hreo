#include "enemybullet.h"

QSize EnemyBullet::defaultSize()
{
    return QSize(120, 40);
}

EnemyBullet::EnemyBullet(const QPointF &startPos,
                         const QPointF &targetPos,
                         int damage,
                         qreal speed,
                         qreal maxDistance)
    : Bullet(startPos,
             targetPos,
             "enemybullet.png",
             speed,
             maxDistance,
             defaultSize())
    , m_damage(damage)
{
}

int EnemyBullet::damage() const
{
    return m_damage;
}

bool EnemyBullet::rotatesToVelocity() const
{
    return true;
}
