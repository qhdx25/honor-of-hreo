#ifndef PET_H
#define PET_H

#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QSize>

class QPainter;

class Pet
{
public:
    Pet();

    void summon(const QPointF &heroCenter);
    void dismiss();
    bool isActive() const;
    void update(const QPointF &heroCenter, qreal deltaMs);
    bool tryShootAt(const QPointF &targetPos, qreal deltaMs);
    QPointF center() const;
    QPointF shootOrigin() const;
    QRectF boundingRect() const;
    void paint(QPainter &painter) const;

private:
    QPointF m_center;
    QPixmap m_pixmap;
    QSize m_size = QSize(96, 96);
    qreal m_orbitAngleRadians = 0.0;
    qreal m_attackCooldownMs = 0.0;
    bool m_active = false;
};

#endif // PET_H
