#ifndef TOWER_H
#define TOWER_H

#include <QPointF>
#include <QPixmap>
#include <QRectF>
#include <QSize>

class QPainter;

class Tower
{
public:
    Tower();

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
    QSize m_size = QSize(170, 250);
    int m_maxHp = 900;
    int m_hp = 900;
    qreal m_attackCooldownMs = 0.0;
};

#endif // TOWER_H
