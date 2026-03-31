#ifndef DRAGONTORNADOBULLET_H
#define DRAGONTORNADOBULLET_H

#include "bullet.h"

#include <QColor>
#include <QVector>

class DragonTornadoBullet : public Bullet
{
public:
    struct ParticleSeed {
        qreal angleRadians = 0.0;
        qreal orbitRadius = 0.0;
        qreal riseOffset = 0.0;
        qreal size = 0.0;
        qreal swirlRate = 0.0;
        qreal riseRate = 0.0;
        QColor color;
    };

    static QSize defaultSize();

    DragonTornadoBullet(const QPointF &startPos,
                        const QPointF &targetPos,
                        int damage,
                        qreal speed = 11.5,
                        qreal maxDistance = 980.0);

    void update() override;
    int damage() const override;
    void paint(QPainter &painter) const override;
    bool isOutOfBounds(int width, int height) const override;
    bool hasReachedMaxDistance() const override;
    QRectF boundingRect() const override;

private:
    QVector<ParticleSeed> buildParticles() const;

    QPointF m_pos;
    QPointF m_velocity;
    QSize m_size;
    QVector<ParticleSeed> m_particles;
    qreal m_distanceTraveled = 0.0;
    qreal m_maxDistance = 980.0;
    qreal m_rotationDegrees = 0.0;
    qreal m_elapsedMs = 0.0;
    int m_damage = 52;
};

#endif // DRAGONTORNADOBULLET_H
