#include "enemy.h"
#include "assetpaths.h"

#include <QPolygonF>
#include <QRectF>
#include <QPainter>
#include <QLineF>
#include <algorithm>

namespace {
QString assetPath(const QString &fileName)
{
    return QString::fromUtf8(kAssetDir) + "/" + fileName;
}
// 功能：传入一个敌人类型 Enemy::Type，返回对应的尺寸 QSize（宽, 高）
QSize enemySizeForType(Enemy::Type type)
{
    switch (type) {
    case Enemy::Type::Scout:// 侦察兵
        return QSize(92, 92);
    case Enemy::Type::Warrior:// 战士
        return QSize(84, 84);
    case Enemy::Type::Mage:// 法师
        return QSize(104, 104);
    case Enemy::Type::Tank:
        return QSize(64, 64);// 坦克
    case Enemy::Type::Assassin:
        return QSize(50, 50);
    }
    return QSize(48, 48);
}
// 作用：传入敌人类型，返回这个敌人的【最大血量】
// 返回值是 int 整数，代表血量数值
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
// 作用：传入敌人类型 Enemy::Type，返回这个敌人的【移动速度】
// 返回值类型 qreal = Qt 里的小数（浮点数）
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
// 作用：传入敌人类型，返回【攻击范围 / 伸手距离】
// qreal = 小数，代表范围半径
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
    }
    return 1000.0;
}
// 函数作用：传入敌人类型，返回【对应的图片】
const QPixmap &enemySpriteForType(Enemy::Type type)
{
    static const QPixmap scoutPixmap(assetPath("hok_ballista_minion.png"));
    static const QPixmap warriorPixmap(assetPath("hok_cannon_minion.png"));
    static const QPixmap magePixmap(assetPath("hok_mage_minion.png"));
    static const QPixmap tankPixmap(assetPath("hok_super_minion.png"));
    static const QPixmap assassinPixmap(assetPath("hok_melee_minion.png"));
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
    }

    return emptyPixmap;
}
} // namespace
// 构造函数：创建一个敌人
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
// 碰撞矩形
QRectF Enemy::boundingRect() const
{
    return QRectF(m_pos.x() - m_size.width() / 2.0,
                  m_pos.y() - m_size.height() / 2.0,
                  m_size.width(),
                  m_size.height());
}
//受伤逻辑
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
//判断敌人是否 “进入屏幕
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
//朝目标移动
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

    const QPixmap &enemySprite = enemySpriteForType(m_type);
    if (!enemySprite.isNull()) {
        painter.drawPixmap(bodyRect.toRect(), enemySprite);
    } else {
        painter.setBrush(bodyColor());
        painter.drawEllipse(bodyRect);
    }

    painter.restore();
}
//判断敌人是否完全跑出了屏幕外
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
//判断敌人是否【已经靠近目标，可以攻击了】
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
//给每种敌人分配一个专属的「强调色」，用于 UI、特效、边框、血条、图标
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
