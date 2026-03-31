#ifndef BOOMERANGBULLET_H
#define BOOMERANGBULLET_H

#include "bullet.h"

#include <QSet>
#include <functional>

class Enemy;

class BoomerangBullet : public Bullet
{
public:
    static QSize defaultSize();

    BoomerangBullet(const QPointF &startPos,
                    const QPointF &targetPos,
                    std::function<QPointF()> returnTargetProvider,
                    qreal speed = 20.0,
                    qreal maxDistance = 900.0);

    void update() override;
    int damage() const override;
    void paint(QPainter &painter) const override;
    bool isOutOfBounds(int width, int height) const override;
    bool hasReachedMaxDistance() const override;
    QRectF boundingRect() const override;

    bool canHitEnemy(const Enemy *enemy) const;
    void registerEnemyHit(const Enemy *enemy);

private:
    QPointF m_pos;
    QPointF m_velocity;
    QPixmap m_pixmap;
    QSize m_size;
    std::function<QPointF()> m_returnTargetProvider;
    QSet<const Enemy *> m_outboundHitEnemies;
    QSet<const Enemy *> m_returnHitEnemies;
    qreal m_speed = 20.0;
    qreal m_distanceTraveled = 0.0;
    qreal m_maxDistance = 900.0;
    qreal m_rotationDegrees = 0.0;
    bool m_returning = false;
    bool m_finished = false;
};

#endif // BOOMERANGBULLET_H
