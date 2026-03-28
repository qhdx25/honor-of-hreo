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
           qreal maxDistance = 420.0,
           const QSize &size = defaultSize());
    virtual ~Bullet() = default;

    void update();
    virtual int damage() const;
    virtual void paint(QPainter &painter) const;
    bool isOutOfBounds(int width, int height) const;
    bool hasReachedMaxDistance() const;
    QRectF boundingRect() const;

protected:
    Bullet(const QPointF &startPos,
           const QPointF &targetPos,
           const QString &spriteFileName,
           qreal speed,
           qreal maxDistance,
           const QSize &size);

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
