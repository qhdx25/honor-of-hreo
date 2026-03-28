#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QSize>

class QPainter;

class Crystal
{
public:
    Crystal();

    void setCenter(const QPointF &center);
    QPointF center() const;
    QPointF shootOrigin() const;
    QRectF boundingRect() const;
    void paint(QPainter &painter) const;
    void takeDamage(int amount);
    void reset();
    bool isDead() const;
    bool tryShootAt(const QPointF &targetPos, qreal deltaMs);
    qreal hpRatio() const;

private:
    QPointF m_center;
    QPixmap m_pixmap;
    QSize m_size = QSize(240, 240);
    int m_maxHp = 1800;
    int m_hp = 1800;
    qreal m_attackCooldownMs = 0.0;
};

#endif // CRYSTAL_H
