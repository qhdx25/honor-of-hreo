#ifndef TOWER_H
#define TOWER_H

#include "config.h"

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
    QSize m_size = QSize(GameConfig::kTowerWidth, GameConfig::kTowerHeight);
    int m_maxHp = GameConfig::kTowerMaxHp;
    int m_hp = GameConfig::kTowerMaxHp;
    qreal m_attackCooldownMs = 0.0;
};

#endif // TOWER_H
