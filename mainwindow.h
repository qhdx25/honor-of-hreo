#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QSet>
#include <QTimer>
#include <QVector>

#include "hero.h"

class SkillIconWidget;
class Bullet;
class Enemy;
class QPainter;
class QMediaPlayer;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initScene();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    enum class SkillType {
        None,
        Skill1,
        Skill2,
        Skill3,
        Flash
    };

    struct Skill2Explosion {
        QPointF center;
        qreal elapsed = 0.0;
    };

    void spawnEnemy();
    void updateBullets();
    void updateEnemies();
    void updateSkillCooldowns();
    void updateSkill2Effects();
    void beginSkillAim(SkillType skill);
    void updateSkillAim(SkillType skill, const QPoint &dragOffset);
    void releaseSkill(SkillType skill, const QPoint &dragOffset);
    void clearSkillAim();
    void castSkill1();
    void castSkill2();
    void castSkill3();
    void castFlash();
    void castTreatment();
    void updateFlashState();
    void updateHeroMovement();
    void updateHeroAnimation();
    void updateSkill3Effect();
    void startGame();
    void setGameplayUiVisible(bool visible);
    QRect startButtonRect() const;
    void drawSkill2Effects(QPainter &painter) const;
    void drawSkillArrow(QPainter &painter) const;
    void drawFlashEffect(QPainter &painter) const;
    void drawHeroHealthBar(QPainter &painter) const;
    void drawSkill3Effect(QPainter &painter) const;
    QPointF cameraOffset() const;
    int worldWidth() const;
    int worldHeight() const;
    QPointF heroCenter() const;

    SkillIconWidget *m_skill1Icon = nullptr;
    SkillIconWidget *m_skill2Icon = nullptr;
    SkillIconWidget *m_skill3Icon = nullptr;
    SkillIconWidget *m_flashIcon = nullptr;
    SkillIconWidget *m_treatmentIcon = nullptr;
    QTimer *m_gameTimer = nullptr;
    QTimer *m_enemyTimer = nullptr;
    QMediaPlayer *m_menuBgmPlayer = nullptr;
    QMediaPlayer *m_bgmPlayer = nullptr;
    hero *myHero = nullptr;
    QVector<Bullet *> m_bullets;
    QVector<Enemy *> m_enemies;
    QVector<Skill2Explosion> m_skill2Explosions;
    QVector<Enemy *> m_skill3HitEnemies;
    QVector<QPixmap> m_heroMoveFrames;
    QSet<int> m_pressedMovementKeys;
    bool m_gameStarted = false;
    SkillType m_activeSkill = SkillType::None;
    bool m_skillAiming = false;
    bool m_heroMoving = false;
    bool m_flashEffectActive = false;
    bool m_skill3Active = false;
    QPointF m_skillDirection = QPointF(1.0, 0.0);
    QPointF m_heroVelocity;
    QPointF m_flashEffectStartPos;
    QPointF m_flashEffectEndPos;
    QPointF m_skill3BaseDirection = QPointF(1.0, 0.0);
    qreal m_skill1CooldownRemainingMs = 0.0;
    qreal m_skill2CooldownRemainingMs = 0.0;
    qreal m_skill3CooldownRemainingMs = 0.0;
    qreal m_flashCooldownRemainingMs = 0.0;
    qreal m_treatmentCooldownRemainingMs = 0.0;
    qreal m_flashEffectElapsed = 0.0;
    qreal m_skillDragLength = 0.0;
    qreal m_heroMoveAnimationElapsed = 0.0;
    qreal m_heroMoveHoldElapsed = 0.0;
    qreal m_skill3Elapsed = 0.0;
    int m_heroMoveFrameIndex = 0;
    QPixmap m_heroIdlePixmap;
    QPixmap m_heroBloodPixmap;
    QPixmap m_mapPixmap;
    QPixmap m_startMenuPixmap;
    QPixmap m_startButtonPixmap;
    QPixmap m_skill3LaserPixmap;
};

#endif // MAINWINDOW_H
