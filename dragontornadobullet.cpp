#include "dragontornadobullet.h"

#include <QLineF>
#include <QPainter>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <cmath>

namespace {
constexpr qreal kPi = 3.14159265358979323846;
}

QSize DragonTornadoBullet::defaultSize()
{
    return QSize(120, 120);
}

DragonTornadoBullet::DragonTornadoBullet(const QPointF &startPos,
                                         const QPointF &targetPos,
                                         int damage,
                                         qreal speed,
                                         qreal maxDistance)
    : Bullet(startPos, targetPos, speed, maxDistance, defaultSize())
    , m_pos(startPos)
    , m_size(defaultSize())
    , m_particles(buildParticles())
    , m_maxDistance(maxDistance)
    , m_damage(damage)
{
    const QLineF line(startPos, targetPos);
    const qreal length = line.length();

    if (length > 0.0001) {
        const QPointF direction((targetPos.x() - startPos.x()) / length,
                                (targetPos.y() - startPos.y()) / length);
        m_velocity = QPointF(direction.x() * speed, direction.y() * speed);
    } else {
        m_velocity = QPointF(speed, 0.0);
    }
}

void DragonTornadoBullet::update()
{
    m_pos += m_velocity;
    m_distanceTraveled += std::hypot(m_velocity.x(), m_velocity.y());
    m_rotationDegrees = std::fmod(m_rotationDegrees + 14.0, 360.0);
    m_elapsedMs += 16.0;
}

int DragonTornadoBullet::damage() const
{
    return m_damage;
}

void DragonTornadoBullet::paint(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    const qreal travelAngleDegrees = std::atan2(m_velocity.y(), m_velocity.x()) * 180.0 / kPi;
    const qreal spinRadians = m_rotationDegrees * kPi / 180.0;
    const qreal pulse = 0.86 + 0.14 * std::sin(m_elapsedMs / 110.0);

    painter.translate(m_pos);
    painter.rotate(travelAngleDegrees);

    QRadialGradient outerGlow(QPointF(0.0, 4.0), 58.0 * pulse);
    outerGlow.setColorAt(0.0, QColor(255, 236, 164, 210));
    outerGlow.setColorAt(0.42, QColor(255, 138, 64, 170));
    outerGlow.setColorAt(1.0, QColor(255, 74, 24, 0));
    painter.setPen(Qt::NoPen);
    painter.setBrush(outerGlow);
    painter.drawEllipse(QRectF(-62.0, -58.0, 124.0, 116.0));

    for (int layer = 0; layer < 5; ++layer) {
        const qreal layerRatio = static_cast<qreal>(layer) / 4.0;
        const qreal twist = spinRadians * (1.2 + layerRatio * 0.45);
        const qreal offsetX = std::sin(twist) * (20.0 - layerRatio * 8.0);
        const qreal offsetY = 36.0 - layer * 20.0;
        const qreal radiusX = 34.0 - layer * 4.0;
        const qreal radiusY = 22.0 - layer * 2.5;

        QRadialGradient layerGradient(QPointF(offsetX, offsetY - 4.0), radiusX * 1.35);
        layerGradient.setColorAt(0.0, QColor(255, 250, 202, 220));
        layerGradient.setColorAt(0.3, QColor(255, 184, 82, 190));
        layerGradient.setColorAt(0.7, QColor(255, 96, 38, 150));
        layerGradient.setColorAt(1.0, QColor(255, 70, 26, 0));
        painter.setBrush(layerGradient);
        painter.drawEllipse(QPointF(offsetX, offsetY), radiusX, radiusY);
    }

    painter.save();
    painter.rotate(m_rotationDegrees);
    QRadialGradient coreGradient(QPointF(0.0, 10.0), 36.0);
    coreGradient.setColorAt(0.0, QColor(255, 251, 215, 245));
    coreGradient.setColorAt(0.35, QColor(255, 210, 98, 220));
    coreGradient.setColorAt(0.75, QColor(255, 104, 40, 165));
    coreGradient.setColorAt(1.0, QColor(255, 70, 22, 0));
    painter.setBrush(coreGradient);
    painter.drawEllipse(QRectF(-32.0, -26.0, 64.0, 82.0));
    painter.restore();

    for (const ParticleSeed &particle : m_particles) {
        const qreal risePhase = std::fmod(m_elapsedMs / 1000.0 * particle.riseRate + particle.riseOffset, 1.0);
        const qreal taper = 1.0 - risePhase * 0.72;
        const qreal orbitAngle = particle.angleRadians + spinRadians * particle.swirlRate;
        const qreal particleX = std::cos(orbitAngle) * particle.orbitRadius * taper;
        const qreal particleY = 36.0 - risePhase * 86.0;
        const qreal particleRadius = particle.size * (0.55 + taper * 0.85);

        QRadialGradient particleGradient(QPointF(particleX, particleY), particleRadius * 1.6);
        QColor centerColor = particle.color;
        centerColor.setAlpha(235);
        QColor edgeColor = particle.color;
        edgeColor.setAlpha(0);
        particleGradient.setColorAt(0.0, centerColor);
        particleGradient.setColorAt(1.0, edgeColor);
        painter.setBrush(particleGradient);
        painter.drawEllipse(QPointF(particleX, particleY), particleRadius, particleRadius);
    }

    painter.restore();
}

bool DragonTornadoBullet::isOutOfBounds(int width, int height) const
{
    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;

    return m_pos.x() + halfWidth < 0.0
        || m_pos.x() - halfWidth > width
        || m_pos.y() + halfHeight < 0.0
        || m_pos.y() - halfHeight > height;
}

bool DragonTornadoBullet::hasReachedMaxDistance() const
{
    return m_distanceTraveled >= m_maxDistance;
}

QRectF DragonTornadoBullet::boundingRect() const
{
    return QRectF(m_pos.x() - m_size.width() / 2.0,
                  m_pos.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

QVector<DragonTornadoBullet::ParticleSeed> DragonTornadoBullet::buildParticles() const
{
    QVector<ParticleSeed> particles;
    particles.reserve(18);

    for (int i = 0; i < 18; ++i) {
        ParticleSeed particle;
        particle.angleRadians = QRandomGenerator::global()->generateDouble() * kPi * 2.0;
        particle.orbitRadius = 12.0 + QRandomGenerator::global()->generateDouble() * 26.0;
        particle.riseOffset = QRandomGenerator::global()->generateDouble();
        particle.size = 3.5 + QRandomGenerator::global()->generateDouble() * 4.5;
        particle.swirlRate = 0.9 + QRandomGenerator::global()->generateDouble() * 1.7;
        particle.riseRate = 0.8 + QRandomGenerator::global()->generateDouble() * 1.4;

        const int paletteIndex = static_cast<int>(QRandomGenerator::global()->bounded(3u));
        if (paletteIndex == 0) {
            particle.color = QColor(255, 246, 196, 220);
        } else if (paletteIndex == 1) {
            particle.color = QColor(255, 182, 74, 220);
        } else {
            particle.color = QColor(255, 102, 38, 220);
        }

        particles.push_back(particle);
    }

    return particles;
}
