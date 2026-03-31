#include "enemy.h"
#include "assetpaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QLineF>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
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

bool isBossEnemyType(Enemy::Type type)
{
    return type == Enemy::Type::Dragon || type == Enemy::Type::Boss2;
}

QSize enemySizeForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return QSize(92, 92);
    case Enemy::Type::Warrior:
        return QSize(104, 104);
    case Enemy::Type::Mage:
        return QSize(124, 124);
    case Enemy::Type::Tank:
        return QSize(104, 104);
    case Enemy::Type::Assassin:
        return QSize(120, 120);
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return QSize(240, 240);
    }

    return QSize(48, 48);
}

int enemyMaxHpForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return 60;
    case Enemy::Type::Warrior:
        return 110;
    case Enemy::Type::Mage:
        return 80;
    case Enemy::Type::Tank:
        return 160;
    case Enemy::Type::Assassin:
        return 70;
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return 1960;
    }

    return 100;
}

qreal enemySpeedForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return 4.6;
    case Enemy::Type::Warrior:
        return 3.6;
    case Enemy::Type::Mage:
        return 3.2;
    case Enemy::Type::Tank:
        return 2.4;
    case Enemy::Type::Assassin:
        return 5.2;
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return 2.5;
    }

    return 3.5;
}

qreal enemyReachRadiusForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return 18.0;
    case Enemy::Type::Warrior:
        return 22.0;
    case Enemy::Type::Mage:
        return 20.0;
    case Enemy::Type::Tank:
        return 26.0;
    case Enemy::Type::Assassin:
        return 19.0;
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return 540.0;
    }

    return 20.0;
}

int enemyAttackDamageForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return 8;
    case Enemy::Type::Warrior:
        return 14;
    case Enemy::Type::Mage:
        return 12;
    case Enemy::Type::Tank:
        return 18;
    case Enemy::Type::Assassin:
        return 16;
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return 52;
    }

    return 10;
}

qreal enemyAttackIntervalForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return 780.0;
    case Enemy::Type::Warrior:
        return 950.0;
    case Enemy::Type::Mage:
        return 1100.0;
    case Enemy::Type::Tank:
        return 1250.0;
    case Enemy::Type::Assassin:
        return 720.0;
    case Enemy::Type::Dragon:
    case Enemy::Type::Boss2:
        return 1750.0;
    }

    return 1000.0;
}

const QPixmap &enemySpriteForType(Enemy::Type type)
{
    static const QPixmap scoutPixmap(assetPath("hok_ballista_minion.png"));
    static const QPixmap warriorPixmap(assetPath("hok_cannon_minion.png"));
    static const QPixmap magePixmap(assetPath("hok_mage_minion.png"));
    static const QPixmap tankPixmap(assetPath("hok_super_minion.png"));
    static const QPixmap assassinPixmap(assetPath("hok_melee_minion.png"));
    static const QPixmap dragonPixmap(assetPath("dragon.png"));
    static const QPixmap boss2Pixmap(assetPath("boss2.png"));
    static const QPixmap emptyPixmap;

    switch (type) {
    case Enemy::Type::Scout:
        return scoutPixmap;
    case Enemy::Type::Warrior:
        return warriorPixmap;
    case Enemy::Type::Mage:
        return magePixmap;
    case Enemy::Type::Tank:
        return tankPixmap;
    case Enemy::Type::Assassin:
        return assassinPixmap;
    case Enemy::Type::Dragon:
        return dragonPixmap;
    case Enemy::Type::Boss2:
        return boss2Pixmap;
    }

    return emptyPixmap;
}
} // namespace

Enemy::Enemy(Type type, const QPointF &startPos)
    : m_pos(startPos)
    , m_type(type)
    , m_size(enemySizeForType(type))
    , m_speed(enemySpeedForType(type))
    , m_reachRadius(enemyReachRadiusForType(type))
    , m_maxHp(enemyMaxHpForType(type))
    , m_hp(m_maxHp)
{
}

QRectF Enemy::boundingRect() const
{
    return QRectF(m_pos.x() - m_size.width() / 2.0,
                  m_pos.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}

void Enemy::takeDamage(int amount)
{
    if (amount <= 0 || m_hp <= 0) {
        return;
    }

    m_hp -= amount;
    if (m_hp < 0) {
        m_hp = 0;
    }
}

void Enemy::updateEnteredState(int width, int height)
{
    if (m_hasEnteredScreen) {
        return;
    }

    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;

    const bool intersects =
        (m_pos.x() + halfWidth >= 0) &&
        (m_pos.x() - halfWidth <= width) &&
        (m_pos.y() + halfHeight >= 0) &&
        (m_pos.y() - halfHeight <= height);

    if (intersects) {
        m_hasEnteredScreen = true;
    }
}

void Enemy::updateToward(const QPointF &targetPos)
{
    const QLineF line(m_pos, targetPos);
    const qreal length = line.length();

    if (length > 0.0001) {
        const QPointF direction((targetPos.x() - m_pos.x()) / length,
                                (targetPos.y() - m_pos.y()) / length);
        m_velocity = QPointF(direction.x() * m_speed, direction.y() * m_speed);
        m_pos += m_velocity;
    }
}

bool Enemy::tryAttackTarget(const QPointF &targetPos, qreal deltaMs)
{
    m_attackCooldownMs = std::max(0.0, m_attackCooldownMs - deltaMs);
    if (!reachesTarget(targetPos) || m_attackCooldownMs > 0.0) {
        return false;
    }

    m_attackCooldownMs = enemyAttackIntervalForType(m_type);
    return true;
}

void Enemy::setCenter(const QPointF &centerPos)
{
    m_pos = centerPos;
}

void Enemy::applyKnockback(const QPointF &direction, qreal distance, int worldWidth, int worldHeight)
{
    if (distance <= 0.0) {
        return;
    }

    const qreal length = std::hypot(direction.x(), direction.y());
    if (length <= 0.0001) {
        return;
    }

    qreal adjustedDistance = distance;
    if (isBossEnemyType(m_type)) {
        adjustedDistance *= 0.4;
    } else if (m_type == Type::Tank) {
        adjustedDistance *= 0.65;
    }

    const QPointF normalizedDirection(direction.x() / length, direction.y() / length);
    const QPointF nextCenter = m_pos + normalizedDirection * adjustedDistance;
    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;
    const qreal maxCenterX = std::max(halfWidth, worldWidth - halfWidth * 1.0);
    const qreal maxCenterY = std::max(halfHeight, worldHeight - halfHeight * 1.0);

    m_pos = QPointF(std::clamp(nextCenter.x(), halfWidth, maxCenterX),
                    std::clamp(nextCenter.y(), halfHeight, maxCenterY));
}

int Enemy::attackDamage() const
{
    return enemyAttackDamageForType(m_type);
}

void Enemy::paint(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    const QRectF bodyRect = boundingRect();
    const bool isBoss = isBossEnemyType(m_type);
    const qreal barHeight = isBoss ? 12.0 : 6.0;
    const qreal barMarginBottom = isBoss ? 16.0 : 8.0;
    const QRectF barBgRect(bodyRect.left(),
                           bodyRect.top() - barMarginBottom - barHeight,
                           bodyRect.width(),
                           barHeight);

    painter.setBrush(QColor(40, 40, 40, 200));
    painter.drawRoundedRect(barBgRect, isBoss ? 5.0 : 2.0, isBoss ? 5.0 : 2.0);

    const qreal hpRatio = (m_maxHp <= 0) ? 0.0 : (static_cast<qreal>(m_hp) / static_cast<qreal>(m_maxHp));
    const QRectF barHpRect(barBgRect.left(),
                           barBgRect.top(),
                           barBgRect.width() * std::clamp(hpRatio, 0.0, 1.0),
                           barBgRect.height());

    painter.setBrush(isBoss ? QColor(255, 92, 44) : QColor(230, 30, 30));
    painter.drawRoundedRect(barHpRect, isBoss ? 5.0 : 2.0, isBoss ? 5.0 : 2.0);

    if (isBoss) {
        painter.setBrush(Qt::NoBrush);
        QPen bossBarPen(QColor(255, 214, 120, 220));
        bossBarPen.setWidth(2);
        painter.setPen(bossBarPen);
        painter.drawRoundedRect(barBgRect.adjusted(-3.0, -3.0, 3.0, 3.0), 7.0, 7.0);
        painter.setPen(Qt::NoPen);
    }

    const QPixmap &enemySprite = enemySpriteForType(m_type);
    if (!enemySprite.isNull()) {
        painter.drawPixmap(bodyRect.toRect(), enemySprite);
    } else {
        painter.setBrush(bodyColor());
        painter.drawEllipse(bodyRect);
    }

    painter.restore();
}

bool Enemy::isOutOfBounds(int width, int height) const
{
    const qreal halfWidth = m_size.width() / 2.0;
    const qreal halfHeight = m_size.height() / 2.0;

    return m_pos.x() + halfWidth < 0
        || m_pos.x() - halfWidth > width
        || m_pos.y() + halfHeight < 0
        || m_pos.y() - halfHeight > height;
}

bool Enemy::reachesTarget(const QPointF &targetPos) const
{
    return QLineF(m_pos, targetPos).length() <= m_reachRadius;
}

QColor Enemy::bodyColor() const
{
    switch (m_type) {
    case Type::Scout:
        return QColor(220, 60, 60);
    case Type::Warrior:
        return QColor(210, 120, 40);
    case Type::Mage:
        return QColor(135, 80, 210);
    case Type::Tank:
        return QColor(70, 150, 95);
    case Type::Assassin:
        return QColor(35, 95, 190);
    case Type::Dragon:
    case Type::Boss2:
        return QColor(166, 54, 42);
    }

    return QColor(200, 80, 80);
}

QColor Enemy::accentColor() const
{
    switch (m_type) {
    case Type::Scout:
        return QColor(255, 220, 180);
    case Type::Warrior:
        return QColor(255, 235, 170);
    case Type::Mage:
        return QColor(240, 220, 255);
    case Type::Tank:
        return QColor(210, 255, 220);
    case Type::Assassin:
        return QColor(220, 240, 255);
    case Type::Dragon:
    case Type::Boss2:
        return QColor(255, 210, 156);
    }

    return QColor(255, 240, 200);
}
