#include "boomerangbullet.h"
#include "assetpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLineF>
#include <QPainter>
#include <QtGlobal>
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
}

QSize BoomerangBullet::defaultSize()
{
    return QSize(176, 176);
}

BoomerangBullet::BoomerangBullet(const QPointF &startPos,
                                 const QPointF &targetPos,
                                 std::function<QPointF()> returnTargetProvider,
                                 qreal speed,
                                 qreal maxDistance)
    : Bullet(startPos, targetPos, speed, maxDistance, defaultSize())
    , m_pos(startPos)
    , m_pixmap(assetPath("boomerang.png"))
    , m_size(defaultSize())
    , m_returnTargetProvider(std::move(returnTargetProvider))
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

void BoomerangBullet::update()
{
    if (m_finished) {
        return;
    }

    if (!m_returning) {
        m_pos += m_velocity;
        m_distanceTraveled += std::hypot(m_velocity.x(), m_velocity.y());
        if (m_distanceTraveled >= m_maxDistance) {
            m_returning = true;
        }
    }

    if (m_returning) {
        const QPointF returnTarget = m_returnTargetProvider ? m_returnTargetProvider() : m_pos;
        const QLineF line(m_pos, returnTarget);
        const qreal length = line.length();
        if (length <= m_speed || length <= 1.0) {
            m_pos = returnTarget;
            m_finished = true;
        } else {
            const QPointF direction((returnTarget.x() - m_pos.x()) / length,
                                    (returnTarget.y() - m_pos.y()) / length);
            m_velocity = QPointF(direction.x() * m_speed, direction.y() * m_speed);
            m_pos += m_velocity;
        }
    }

    m_rotationDegrees = std::fmod(m_rotationDegrees + 30.0, 360.0);
}

int BoomerangBullet::damage() const
{
    return 55;
}

void BoomerangBullet::paint(QPainter &painter) const
{
    const QRectF rect = boundingRect();

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.translate(rect.center());
    painter.rotate(m_rotationDegrees);

    if (!m_pixmap.isNull()) {
        painter.drawPixmap(QRectF(-m_size.width() / 2.0,
                                  -m_size.height() / 2.0,
                                  m_size.width(),
                                  m_size.height()),
                           m_pixmap,
                           QRectF(0.0, 0.0, m_pixmap.width(), m_pixmap.height()));
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 193, 76, 220));
        painter.drawEllipse(QRectF(-m_size.width() / 2.0,
                                   -m_size.height() / 2.0,
                                   m_size.width(),
                                   m_size.height()));
    }

    painter.restore();
}

bool BoomerangBullet::isOutOfBounds(int width, int height) const
{
    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;

    return m_pos.x() + halfWidth < 0.0
        || m_pos.x() - halfWidth > width
        || m_pos.y() + halfHeight < 0.0
        || m_pos.y() - halfHeight > height;
}

bool BoomerangBullet::hasReachedMaxDistance() const
{
    return m_finished;
}

QRectF BoomerangBullet::boundingRect() const
{
    return QRectF(m_pos.x() - m_size.width() / 2.0,
                  m_pos.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

bool BoomerangBullet::canHitEnemy(const Enemy *enemy) const
{
    if (enemy == nullptr) {
        return false;
    }

    return m_returning
        ? !m_returnHitEnemies.contains(enemy)
        : !m_outboundHitEnemies.contains(enemy);
}

void BoomerangBullet::registerEnemyHit(const Enemy *enemy)
{
    if (enemy == nullptr) {
        return;
    }

    if (m_returning) {
        m_returnHitEnemies.insert(enemy);
    } else {
        m_outboundHitEnemies.insert(enemy);
    }
}
