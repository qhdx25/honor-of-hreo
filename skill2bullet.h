#ifndef SKILL2BULLET_H
#define SKILL2BULLET_H

#include "bullet.h"

class Skill2Bullet : public Bullet
{
public:
    static QSize defaultSize();

    Skill2Bullet(const QPointF &startPos,
                 const QPointF &targetPos,
                 qreal speed = GameConfig::kSkill2BulletDefaultSpeed,
                 qreal maxDistance = GameConfig::kSkill2BulletDefaultMaxDistance);

    int damage() const override;
};

#endif // SKILL2BULLET_H
