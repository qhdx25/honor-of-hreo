#include "crystal.h"

#include "assetpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QString>
#include <algorithm>

namespace {
QString assetPath(const QString &fileName)
{
    const QString packagedPath = QDir(QCoreApplication::applicationDirPath()).filePath("res/" + fileName);
    if (QFileInfo::exists(packagedPath)) {
        return packagedPath;
    }

    return QString::fromUtf8(kSourceAssetDir) + "/" + fileName;
}

constexpr qreal kCrystalAttackIntervalMs = 1800.0;
constexpr int kCrystalHpBarHeight = 12;
}

Crystal::Crystal()
    : m_pixmap(assetPath("crystal.png"))
{
}

void Crystal::setCenter(const QPointF &center)
{
    m_center = center;
}

QPointF Crystal::center() const
{
    return m_center;
}

QPointF Crystal::shootOrigin() const
{
    return m_center;
}

QRectF Crystal::boundingRect() const
{
    return QRectF(m_center.x() - m_size.width() / 2.0,
                  m_center.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

void Crystal::paint(QPainter &painter) const
{
    if (isDead()) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF bodyRect = boundingRect();
    const QRectF barBgRect(bodyRect.left() + 10.0,
                           bodyRect.top() - 22.0,
                           bodyRect.width() - 20.0,
                           kCrystalHpBarHeight);
    const QRectF barFillRect(barBgRect.left(),
                             barBgRect.top(),
                             barBgRect.width() * std::clamp(hpRatio(), 0.0, 1.0),
                             barBgRect.height());

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 45, 45, 220));
    painter.drawRoundedRect(barBgRect, 4, 4);
    painter.setBrush(QColor(215, 54, 54, 235));
    painter.drawRoundedRect(barFillRect, 4, 4);

    if (!m_pixmap.isNull()) {
        painter.drawPixmap(bodyRect.toRect(), m_pixmap);
    } else {
        painter.setBrush(QColor(126, 198, 255, 220));
        painter.drawEllipse(bodyRect);
    }

    painter.restore();
}

void Crystal::takeDamage(int amount)
{
    if (amount <= 0 || isDead()) {
        return;
    }

    m_hp = std::max(0, m_hp - amount);
}

void Crystal::reset()
{
    m_hp = m_maxHp;
    m_attackCooldownMs = 0.0;
}

bool Crystal::isDead() const
{
    return m_hp <= 0;
}

bool Crystal::tryShootAt(const QPointF &targetPos, qreal deltaMs)
{
    if (isDead()) {
        return false;
    }

    m_attackCooldownMs = std::max(0.0, m_attackCooldownMs - deltaMs);
    if (QLineF(m_center, targetPos).length() <= 1.0 || m_attackCooldownMs > 0.0) {
        return false;
    }

    m_attackCooldownMs = kCrystalAttackIntervalMs;
    return true;
}

qreal Crystal::hpRatio() const
{
    if (m_maxHp <= 0) {
        return 0.0;
    }

    return static_cast<qreal>(m_hp) / static_cast<qreal>(m_maxHp);
}
