#include "enemy.h"

#include <QPolygonF>
#include <QRectF>
#include <QPainter>
#include <QLineF>
#include <algorithm>

namespace {
QSize enemySizeForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:
        return QSize(44, 44);
    case Enemy::Type::Warrior:
        return QSize(54, 54);
    case Enemy::Type::Mage:
        return QSize(48, 48);
    case Enemy::Type::Tank:
        return QSize(64, 64);
    case Enemy::Type::Assassin:
        return QSize(50, 50);
    }
    return QSize(48, 48);
}

int enemyMaxHpForType(Enemy::Type type)
{
    // 你可以按玩法随时调整这些数值
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
    }
    return 20.0;
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

    // “进入屏幕”的判定：敌人整体至少有一部分在窗口范围内即可。
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

void Enemy::paint(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    const QRectF bodyRect = boundingRect();

    // 头顶血条（红色代表生命值）
    // - 血条随敌人一起移动：只要用 bodyRect 计算位置即可
    // - 当 hp 变为 0，会在上层逻辑中删除敌人
    const qreal barHeight = 6.0;
    const qreal barMarginBottom = 8.0; // 血条距离敌人顶部的间距
    const QRectF barBgRect(bodyRect.left(),
                           bodyRect.top() - barMarginBottom - barHeight,
                           bodyRect.width(),
                           barHeight);

    painter.setBrush(QColor(40, 40, 40, 200)); // 血条底色（暗灰）
    painter.drawRoundedRect(barBgRect, 2, 2);

    const qreal hpRatio = (m_maxHp <= 0) ? 0.0 : (static_cast<qreal>(m_hp) / static_cast<qreal>(m_maxHp));
    const QRectF barHpRect(barBgRect.left(),
                           barBgRect.top(),
                           barBgRect.width() * std::clamp(hpRatio, 0.0, 1.0),
                           barBgRect.height());

    painter.setBrush(QColor(230, 30, 30)); // 红色血量条
    painter.drawRoundedRect(barHpRect, 2, 2);

    painter.setBrush(bodyColor());
    painter.drawEllipse(bodyRect);

    painter.setBrush(accentColor());

    switch (m_type) {
    case Type::Scout:
        painter.drawEllipse(bodyRect.adjusted(m_size.width() * 0.18,
                                              m_size.height() * 0.18,
                                              -m_size.width() * 0.18,
                                              -m_size.height() * 0.18));
        break;
    case Type::Warrior:
        painter.drawRect(bodyRect.adjusted(m_size.width() * 0.22,
                                           m_size.height() * 0.34,
                                           -m_size.width() * 0.22,
                                           -m_size.height() * 0.18));
        break;
    case Type::Mage:
        painter.drawRoundedRect(bodyRect.adjusted(m_size.width() * 0.20,
                                                  m_size.height() * 0.16,
                                                  -m_size.width() * 0.20,
                                                  -m_size.height() * 0.16),
                                8,
                                8);
        break;
    case Type::Tank:
        painter.drawEllipse(bodyRect.adjusted(m_size.width() * 0.28,
                                              m_size.height() * 0.28,
                                              -m_size.width() * 0.28,
                                              -m_size.height() * 0.28));
        break;
    case Type::Assassin:
    {
        QPolygonF polygon;
        polygon << QPointF(bodyRect.center().x(), bodyRect.top() + 4)
                << QPointF(bodyRect.right() - 4, bodyRect.center().y())
                << QPointF(bodyRect.center().x(), bodyRect.bottom() - 4)
                << QPointF(bodyRect.left() + 4, bodyRect.center().y());
        painter.drawPolygon(polygon);
        break;
    }
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
    }
    return QColor(255, 240, 200);
}
