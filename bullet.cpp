#include "bullet.h"
#include "assetpaths.h"

#include <QLineF>
#include <QPainter>
#include <cmath>

namespace {
QString assetPath(const QString &fileName)
{
    return QString::fromUtf8(kAssetDir) + "/" + fileName;
}
}

QSize Bullet::defaultSize()
{
    return QSize(76, 76);
}

Bullet::Bullet(const QPointF &startPos,
               const QPointF &targetPos,
               qreal speed,
               qreal maxDistance,
               const QSize &size)
    : Bullet(startPos,
             targetPos,
             "angela_basic_attack.png",
             speed,
             maxDistance,
             size)
{
}

Bullet::Bullet(const QPointF &startPos,
               const QPointF &targetPos,
               const QString &spriteFileName,
               qreal speed,
               qreal maxDistance,
               const QSize &size)
    : m_pos(startPos)
    , m_pixmap(assetPath(spriteFileName))
    , m_size(size)
    , m_speed(speed)
    , m_maxDistance(maxDistance)
{
    const QLineF line(startPos, targetPos);
    const qreal length = line.length();

    if (length > 0.0001) {
        const QPointF direction((targetPos.x() - startPos.x()) / length,
                                (targetPos.y() - startPos.y()) / length);
        m_velocity = QPointF(direction.x() * m_speed, direction.y() * m_speed);
    } else {
        m_velocity = QPointF(m_speed, 0.0);
    }
}

void Bullet::update()
{
    m_distanceTraveled += std::hypot(m_velocity.x(), m_velocity.y());
    m_pos += m_velocity;
}

int Bullet::damage() const
{
    return 25;
}

void Bullet::paint(QPainter &painter) const
{
    if (m_pixmap.isNull()) {
        return;
    }

    painter.drawPixmap(static_cast<int>(m_pos.x() - m_size.width() / 2.0),
                       static_cast<int>(m_pos.y() - m_size.height() / 2.0),
                       m_size.width(),
                       m_size.height(),
                       m_pixmap);
}

bool Bullet::isOutOfBounds(int width, int height) const
{
    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;

    return m_pos.x() + halfWidth < 0.0
        || m_pos.x() - halfWidth > width
        || m_pos.y() + halfHeight < 0.0
        || m_pos.y() - halfHeight > height;
}

bool Bullet::hasReachedMaxDistance() const
{
    return m_distanceTraveled >= m_maxDistance;
}

QRectF Bullet::boundingRect() const
{
    return QRectF(m_pos.x() - m_size.width() / 2.0,
                  m_pos.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}
