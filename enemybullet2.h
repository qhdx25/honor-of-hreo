#ifndef ENEMYBULLET2_H
#define ENEMYBULLET2_H

#include "bullet.h"

class EnemyBullet2 : public Bullet
{
public:
    static QSize defaultSize();

    EnemyBullet2(const QPointF &startPos,
                 const QPointF &targetPos,
                 int damage = GameConfig::kEnemyBullet2DefaultDamage,
                 qreal speed = GameConfig::kEnemyBullet2DefaultSpeed,
                 qreal maxDistance = GameConfig::kEnemyBullet2DefaultMaxDistance);

    int damage() const override;

protected:
    bool rotatesToVelocity() const override;

private:
    int m_damage = GameConfig::kEnemyBullet2DefaultDamage;
};

#endif // ENEMYBULLET2_H
