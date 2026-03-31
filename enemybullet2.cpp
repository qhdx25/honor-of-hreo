#include "enemybullet2.h"

QSize EnemyBullet2::defaultSize()
{
    return QSize(132, 44);
}

EnemyBullet2::EnemyBullet2(const QPointF &startPos,
                           const QPointF &targetPos,
                           int damage,
                           qreal speed,
                           qreal maxDistance)
    : Bullet(startPos,
             targetPos,
             "enemybullet2.png",
             speed,
             maxDistance,
             defaultSize())
    , m_damage(damage)
{
}

int EnemyBullet2::damage() const
{
    return m_damage;
}

bool EnemyBullet2::rotatesToVelocity() const
{
    return true;
}
