#ifndef ENEMY_H
#define ENEMY_H

#include <QPointF>
#include <QSize>
#include <QColor>
#include <QRectF>

class QPainter;

class Enemy
{
public:
    enum class Type {
        Scout = 0,
        Warrior,
        Mage,
        Tank,
        Assassin,
        Shooter,
        Dragon,
        Boss2,
        Boss3
    };

    Enemy(Type type, const QPointF &startPos);
    virtual ~Enemy() = default;

    Type type() const { return m_type; }
    void updateToward(const QPointF &targetPos);
    void paint(QPainter &painter) const;
    int attackDamage() const;
    bool tryAttackTarget(const QPointF &targetPos, qreal deltaMs);
    void setCenter(const QPointF &centerPos);
    void applyKnockback(const QPointF &direction, qreal distance, int worldWidth, int worldHeight);

    // 供碰撞检测使用：返回敌人的包围盒（与绘制的 bodyRect 对齐）
    QRectF boundingRect() const;

    // 血量机制
    void takeDamage(int amount);
    bool isDead() const { return m_hp <= 0; }

    bool isOutOfBounds(int width, int height) const;
    bool reachesTarget(const QPointF &targetPos) const;

    // 标记是否已经进入过屏幕（用于“允许从屏幕外生成”）
    // 如果敌人从屏幕外生成（例如右侧外），第一次 update 时仍然 out-of-bounds，
    // 但我们不希望立刻删掉它；等它真正进入屏幕后，后续再出界才清理。
    void updateEnteredState(int width, int height);
    bool hasEnteredScreen() const { return m_hasEnteredScreen; }

private:
    QPointF m_pos;
    QPointF m_velocity;
    Type m_type;
    QSize m_size;
    qreal m_speed;
    qreal m_reachRadius;
    bool m_hasEnteredScreen = false;

    int m_maxHp = 100;
    int m_hp = 100;
    qreal m_attackCooldownMs = 0.0;

    QColor bodyColor() const;
    QColor accentColor() const;
};

#endif // ENEMY_H
