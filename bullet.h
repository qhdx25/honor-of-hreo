#ifndef BULLET_H
#define BULLET_H

#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QSize>
#include <QString>

class QPainter;

class Bullet
{
public:
    static QSize defaultSize();

    Bullet(const QPointF &startPos,
           const QPointF &targetPos,
           qreal speed = 18.0,
           qreal maxDistance = 650.0,
           const QSize &size = defaultSize());
    virtual ~Bullet() = default;

    virtual void update();
    virtual int damage() const;
    virtual void paint(QPainter &painter) const;
    virtual bool isOutOfBounds(int width, int height) const;
    virtual bool hasReachedMaxDistance() const;
    virtual QRectF boundingRect() const;
    QPointF velocity() const { return m_velocity; }

protected:
    Bullet(const QPointF &startPos,
           const QPointF &targetPos,
           const QString &spriteFileName,
           qreal speed,
           qreal maxDistance,
           const QSize &size);
    virtual bool rotatesToVelocity() const;

private:
    QPointF m_pos;
    QPointF m_velocity;
    QPixmap m_pixmap;
    QSize m_size;
    qreal m_speed = 18.0;
    qreal m_distanceTraveled = 0.0;
    qreal m_maxDistance = 360.0;
};

#endif // BULLET_H
