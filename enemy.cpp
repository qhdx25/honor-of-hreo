#include "enemy.h"
#include "config.h"

#include <QLineF>
#include <QPainter>
#include <QPixmap>
#include <QRectF>
#include <algorithm>
#include <cmath>

namespace {
std::size_t enemyTypeIndex(Enemy::Type type)
{
    return static_cast<std::size_t>(type);
}

QString assetPath(const QString &fileName)
{
    return QStringLiteral("D:/develop/Qtproject/honor-of-hero/res/") + fileName;
}

bool isBossEnemyType(Enemy::Type type)
{
    return type == Enemy::Type::Dragon || type == Enemy::Type::Boss2 || type == Enemy::Type::Boss3;
}

bool isRangedEnemyType(Enemy::Type type)
{
    return type == Enemy::Type::Shooter || type == Enemy::Type::Boss3;
}

QSize enemySizeForType(Enemy::Type type)
{
    const std::size_t index = enemyTypeIndex(type);
    return QSize(GameConfig::kEnemyWidthByType.at(index), GameConfig::kEnemyHeightByType.at(index));
}

int enemyMaxHpForType(Enemy::Type type)
{
    return GameConfig::kEnemyMaxHpByType.at(enemyTypeIndex(type));
}

qreal enemySpeedForType(Enemy::Type type)
{
    return GameConfig::kEnemySpeedByType.at(enemyTypeIndex(type));
}

qreal enemyReachRadiusForType(Enemy::Type type)
{
    return GameConfig::kEnemyReachRadiusByType.at(enemyTypeIndex(type));
}

int enemyAttackDamageForType(Enemy::Type type)
{
    return GameConfig::kEnemyAttackDamageByType.at(enemyTypeIndex(type));
}

qreal enemyAttackIntervalForType(Enemy::Type type)
{
    return GameConfig::kEnemyAttackIntervalMsByType.at(enemyTypeIndex(type));
}

const QPixmap &enemySpriteForType(Enemy::Type type)
{
    static const QPixmap scoutPixmap(assetPath("hok_ballista_minion.png"));
    static const QPixmap warriorPixmap(assetPath("hok_cannon_minion.png"));
    static const QPixmap magePixmap(assetPath("hok_mage_minion.png"));
    static const QPixmap tankPixmap(assetPath("hok_super_minion.png"));
    static const QPixmap assassinPixmap(assetPath("hok_melee_minion.png"));
    static const QPixmap shooterPixmap(assetPath("enemy.png"));
    static const QPixmap dragonPixmap(assetPath("dragon.png"));
    static const QPixmap boss2Pixmap(assetPath("boss2.png"));
    static const QPixmap boss3Pixmap(assetPath("boss3.png"));
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
    case Enemy::Type::Shooter:
        return shooterPixmap;
    case Enemy::Type::Dragon:
        return dragonPixmap;
    case Enemy::Type::Boss2:
        return boss2Pixmap;
    case Enemy::Type::Boss3:
        return boss3Pixmap;
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
        adjustedDistance *= GameConfig::kBossKnockbackMultiplier;
    } else if (m_type == Type::Tank) {
        adjustedDistance *= GameConfig::kTankKnockbackMultiplier;
    } else if (isRangedEnemyType(m_type)) {
        adjustedDistance *= GameConfig::kRangedEnemyKnockbackMultiplier;
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
        if (m_type == Type::Shooter && std::abs(m_velocity.x()) > 0.01) {
            painter.save();
            if (m_velocity.x() < 0.0) {
                painter.translate(bodyRect.left(), bodyRect.top());
                painter.scale(1.0, 1.0);
                painter.drawPixmap(QRectF(0.0, 0.0, bodyRect.width(), bodyRect.height()),
                                   enemySprite,
                                   QRectF(0.0, 0.0, enemySprite.width(), enemySprite.height()));
            } else {
                painter.translate(bodyRect.right(), bodyRect.top());
                painter.scale(-1.0, 1.0);
                painter.drawPixmap(QRectF(0.0, 0.0, bodyRect.width(), bodyRect.height()),
                                   enemySprite,
                                   QRectF(0.0, 0.0, enemySprite.width(), enemySprite.height()));
            }
            painter.restore();
        } else {
            painter.drawPixmap(bodyRect.toRect(), enemySprite);
        }
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
    case Type::Shooter:
        return QColor(132, 58, 162);
    case Type::Dragon:
    case Type::Boss2:
    case Type::Boss3:
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
    case Type::Shooter:
        return QColor(245, 210, 255);
    case Type::Dragon:
    case Type::Boss2:
    case Type::Boss3:
        return QColor(255, 210, 156);
    }

    return QColor(255, 240, 200);
}
