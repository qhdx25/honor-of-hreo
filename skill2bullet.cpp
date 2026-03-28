#include "skill2bullet.h"

QSize Skill2Bullet::defaultSize()
{
    const QSize basicSize = Bullet::defaultSize();
    return QSize(basicSize.width() * 2, basicSize.height() * 2);
}

Skill2Bullet::Skill2Bullet(const QPointF &startPos,
                           const QPointF &targetPos,
                           qreal speed,
                           qreal maxDistance)
    : Bullet(startPos,
             targetPos,
             "angela_skill2.png",
             speed,
             maxDistance,
             defaultSize())
{
}

int Skill2Bullet::damage() const
{
    return Bullet::damage() * 4;
}
