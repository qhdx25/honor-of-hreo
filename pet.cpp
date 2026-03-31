#include "pet.h"

#include "assetpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QPainter>
#include <QString>
#include <algorithm>
#include <cmath>

namespace {
QString assetPath(const QString &fileName)
{
    const QString packagedPath = QDir(QCoreApplication::applicationDirPath()).filePath("res/" + fileName);
    if (QFileInfo::exists(packagedPath)) {
        return packagedPath;
    }

    return QString::fromUtf8(kSourceAssetDir) + "/" + fileName;
}

constexpr qreal kPetOrbitRadius = 132.0;
constexpr qreal kPetOrbitAngularSpeed = 0.0038;
constexpr qreal kPetAttackIntervalMs = 1000.0;
}

Pet::Pet()
    : m_pixmap(assetPath("cuteevil.png"))
{
}

void Pet::summon(const QPointF &heroCenter)
{
    m_active = true;
    m_orbitAngleRadians = 0.0;
    m_attackCooldownMs = 0.0;
    m_center = heroCenter + QPointF(kPetOrbitRadius, -32.0);
}

void Pet::dismiss()
{
    m_active = false;
    m_attackCooldownMs = 0.0;
}

bool Pet::isActive() const
{
    return m_active;
}

void Pet::update(const QPointF &heroCenter, qreal deltaMs)
{
    if (!m_active) {
        return;
    }

    m_orbitAngleRadians = std::fmod(m_orbitAngleRadians + deltaMs * kPetOrbitAngularSpeed, 6.28318530717958647692);
    m_center = heroCenter + QPointF(std::cos(m_orbitAngleRadians) * kPetOrbitRadius,
                                    std::sin(m_orbitAngleRadians) * (kPetOrbitRadius * 0.68) - 26.0);
}

bool Pet::tryShootAt(const QPointF &targetPos, qreal deltaMs)
{
    if (!m_active) {
        return false;
    }

    m_attackCooldownMs = std::max(0.0, m_attackCooldownMs - deltaMs);
    const QPointF delta = targetPos - m_center;
    if (std::hypot(delta.x(), delta.y()) <= 1.0 || m_attackCooldownMs > 0.0) {
        return false;
    }

    m_attackCooldownMs = kPetAttackIntervalMs;
    return true;
}

QPointF Pet::center() const
{
    return m_center;
}

QPointF Pet::shootOrigin() const
{
    return m_center;
}

QRectF Pet::boundingRect() const
{
    return QRectF(m_center.x() - m_size.width() / 2.0,
                  m_center.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

void Pet::paint(QPainter &painter) const
{
    if (!m_active) {
        return;
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    const QRectF rect = boundingRect();

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(145, 104, 255, 64));
    painter.drawEllipse(rect.adjusted(12.0, 18.0, -12.0, 24.0));

    if (!m_pixmap.isNull()) {
        painter.drawPixmap(rect.toRect(), m_pixmap);
    } else {
        painter.setBrush(QColor(255, 132, 180, 220));
        painter.drawEllipse(rect);
    }

    painter.restore();
}
