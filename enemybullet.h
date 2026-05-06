#ifndef ENEMYBULLET_H
#define ENEMYBULLET_H

#include "bullet.h"

class EnemyBullet : public Bullet
{
public:
    static QSize defaultSize();

    EnemyBullet(const QPointF &startPos,
                const QPointF &targetPos,
                int damage = GameConfig::kEnemyBulletDefaultDamage,
                qreal speed = GameConfig::kEnemyBulletDefaultSpeed,
                qreal maxDistance = GameConfig::kEnemyBulletDefaultMaxDistance);

    int damage() const override;

protected:
    bool rotatesToVelocity() const override;

private:
    int m_damage = GameConfig::kEnemyBulletDefaultDamage;
};

#endif // ENEMYBULLET_H
