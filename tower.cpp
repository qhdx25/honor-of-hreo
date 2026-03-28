#include "tower.h"

#include "assetpaths.h"

#include <QLineF>
#include <QPainter>
#include <QString>
#include <algorithm>

namespace {
QString assetPath(const QString &fileName)
{
    return QString::fromUtf8(kAssetDir) + "/" + fileName;
}

constexpr qreal kTowerAttackIntervalMs = 2200.0;
constexpr int kTowerHpBarHeight = 10;
}

Tower::Tower()
    : m_pixmap(assetPath("tower.png"))
{
}

void Tower::setCenter(const QPointF &center)
{
    m_center = center;
}

QPointF Tower::center() const
{
    return m_center;
}

QPointF Tower::shootOrigin() const
{
    return QPointF(m_center.x(), m_center.y() - m_size.height() * 0.12);
}

QRectF Tower::boundingRect() const
{
    return QRectF(m_center.x() - m_size.width() / 2.0,
                  m_center.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

void Tower::paint(QPainter &painter) const
{
    if (isDead()) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bodyRect = boundingRect();
    const QRectF barBgRect(bodyRect.left() + 8.0,
                           bodyRect.top() - 18.0,
                           bodyRect.width() - 16.0,
                           kTowerHpBarHeight);
    const QRectF barFillRect(barBgRect.left(),
                             barBgRect.top(),
                             barBgRect.width() * std::clamp(hpRatio(), 0.0, 1.0),
                             barBgRect.height());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 45, 45, 220));
    painter.drawRoundedRect(barBgRect, 4, 4);
    painter.setBrush(QColor(225, 55, 55, 235));
    painter.drawRoundedRect(barFillRect, 4, 4);

    if (!m_pixmap.isNull()) {
        painter.drawPixmap(bodyRect.toRect(), m_pixmap);
    } else {
        painter.setBrush(QColor(205, 165, 108, 220));
        painter.drawRoundedRect(bodyRect, 18, 18);
    }

    painter.restore();
}

void Tower::takeDamage(int amount)
{
    if (amount <= 0 || isDead()) {
        return;
    }

    m_hp = std::max(0, m_hp - amount);
}

void Tower::reset()
{
    m_hp = m_maxHp;
    m_attackCooldownMs = 0.0;
}

bool Tower::isDead() const
{
    return m_hp <= 0;
}

bool Tower::tryShootAt(const QPointF &targetPos, qreal deltaMs)
{
    if (isDead()) {
        return false;
    }

    m_attackCooldownMs = std::max(0.0, m_attackCooldownMs - deltaMs);
    if (QLineF(m_center, targetPos).length() <= 1.0 || m_attackCooldownMs > 0.0) {
        return false;
    }

    m_attackCooldownMs = kTowerAttackIntervalMs;
    return true;
}

qreal Tower::hpRatio() const
{
    if (m_maxHp <= 0) {
        return 0.0;
    }

    return static_cast<qreal>(m_hp) / static_cast<qreal>(m_maxHp);
}
