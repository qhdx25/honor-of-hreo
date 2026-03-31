#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QPixmap>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QVector>

#include "hero.h"

class SkillIconWidget;
class Bullet;
class Enemy;
class Crystal;
class Tower;
class Pet;
class MedicinePack;
class QPainter;
class QMediaPlayer;
class QPushButton;
class QWheelEvent;

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
    void wheelEvent(QWheelEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    enum class SkillType {
        None,
        Skill1,
        Skill2,
        Skill3,
        Skill6,
        Skill7,
        Flash
    };

    struct Skill2Explosion {
        QPointF center;
        qreal elapsed = 0.0;
    };

    struct BulletWheelBurst {
        QPointF center;
        qreal elapsed = 0.0;
    };

    struct DragonAttackWave {
        QPointF center;
        qreal elapsed = 0.0;
        qreal rotationSeed = 0.0;
        qreal scale = 1.0;
    };

    struct DragonDeathBurst {
        QPointF center;
        qreal elapsed = 0.0;
    };

    void spawnEnemy();
    void updateBullets();
    void updateEnemies();
    void updateSkillCooldowns();
    void updateSkill2Effects();
    void updateBulletWheelEffects();
    void updateDragonEffects();
    void beginSkillAim(SkillType skill);
    void updateSkillAim(SkillType skill, const QPoint &dragOffset);
    void releaseSkill(SkillType skill, const QPoint &dragOffset);
    void clearSkillAim();
    void castSkill1();
    void castSkill2();
    void castSkill3();
    void applySkill2AreaDamage(const QPointF &impactCenter, const QPointF &impactDirection, int damage);
    void castSkill6();
    void castSkill7();
    void castBulletWheel();
    void castFlash();
    void castTreatment();
    void updateFlashState();
    void updateHeroMovement();
    void updateHeroAnimation();
    void updateSkill3Effect();
    void playSkillReadySound();
    void startGame();
    void startDefeatSequence();
    void advanceDefeatFrame();
    void startVictorySequence();
    void advanceVictoryFrame();
    void returnToMainMenu();
    void resetGameplayState();
    void setGameplayUiVisible(bool visible);
    QRect startButtonRect() const;
    QRect aboutButtonRect() const;
    QRect iconButtonRect() const;
    QRect aboutBackButtonRect() const;
    QRectF aboutPagePanelRect() const;
    QRectF aboutPageTextViewportRect() const;
    QString aboutPageText() const;
    qreal aboutPageMaxScroll() const;
    void drawSkill2Effects(QPainter &painter) const;
    void drawBulletWheelEffects(QPainter &painter) const;
    void drawDragonEffects(QPainter &painter) const;
    void drawSkillArrow(QPainter &painter) const;
    void drawFlashEffect(QPainter &painter) const;
    void drawHeroHealthBar(QPainter &painter) const;
    void drawSkill3Effect(QPainter &painter) const;
    void spawnDragonEnemy();
    void spawnBoss2Enemy();
    void spawnBoss3Enemy();
    void resolveEnemyOverlap();
    void togglePause();
    void setPaused(bool paused);
    void handleEnemyDefeat(int enemyIndex);
    void playDragonSpawnSound();
    void playDragonDeathSound();
    void updateMedicinePack();
    QPointF cameraOffset() const;
    int worldWidth() const;
    int worldHeight() const;
    QPointF heroCenter() const;

    SkillIconWidget *m_skill1Icon = nullptr;
    SkillIconWidget *m_skill2Icon = nullptr;
    SkillIconWidget *m_skill3Icon = nullptr;
    SkillIconWidget *m_skill6Icon = nullptr;
    SkillIconWidget *m_skill7Icon = nullptr;
    SkillIconWidget *m_flashIcon = nullptr;
    SkillIconWidget *m_treatmentIcon = nullptr;
    QTimer *m_gameTimer = nullptr;
    QTimer *m_enemyTimer = nullptr;
    QTimer *m_defeatFrameTimer = nullptr;
    QTimer *m_victoryFrameTimer = nullptr;
    QMediaPlayer *m_menuBgmPlayer = nullptr;
    QMediaPlayer *m_bgmPlayer = nullptr;
    QMediaPlayer *m_defeatAudioPlayer = nullptr;
    QMediaPlayer *m_victoryAudioPlayer = nullptr;
    QMediaPlayer *m_skill2HitPlayer = nullptr;
    QMediaPlayer *m_heroVoicePlayer = nullptr;
    QMediaPlayer *m_skill3VoicePlayer = nullptr;
    QMediaPlayer *m_skillReadyPlayer = nullptr;
    QMediaPlayer *m_dragonSpawnPlayer = nullptr;
    QMediaPlayer *m_dragonDeathPlayer = nullptr;
    QPushButton *m_pauseButton = nullptr;
    hero *myHero = nullptr;
    Pet *m_pet = nullptr;
    Crystal *m_crystal = nullptr;
    MedicinePack *m_medicinePack = nullptr;
    QVector<Tower *> m_towers;
    QVector<Bullet *> m_bullets;
    QVector<Bullet *> m_enemyBullets;
    QVector<Bullet *> m_petBullets;
    QVector<Enemy *> m_enemies;
    QVector<Skill2Explosion> m_skill2Explosions;
    QVector<BulletWheelBurst> m_bulletWheelBursts;
    QVector<DragonAttackWave> m_dragonAttackWaves;
    QVector<DragonDeathBurst> m_dragonDeathBursts;
    QVector<Enemy *> m_skill3HitEnemies;
    QVector<QPixmap> m_heroMoveFrames;
    QSet<int> m_pressedMovementKeys;
    bool m_gameStarted = false;
    bool m_showAboutPage = false;
    bool m_showIconPage = false;
    bool m_defeatSequenceActive = false;
    bool m_victorySequenceActive = false;
    SkillType m_activeSkill = SkillType::None;
    bool m_skillAiming = false;
    bool m_gamePaused = false;
    bool m_heroMoving = false;
    bool m_heroFacingLeft = false;
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
    qreal m_skill6CooldownRemainingMs = 0.0;
    qreal m_flashCooldownRemainingMs = 0.0;
    qreal m_treatmentCooldownRemainingMs = 0.0;
    qreal m_flashEffectElapsed = 0.0;
    qreal m_skillDragLength = 0.0;
    qreal m_heroMoveAnimationElapsed = 0.0;
    qreal m_heroMoveHoldElapsed = 0.0;
    qreal m_heroVoiceCountdownMs = -1.0;
    qreal m_dragonSpawnCountdownMs = 0.0;
    qreal m_boss3SpawnCountdownMs = 0.0;
    qreal m_medicineRespawnCountdownMs = 0.0;
    qreal m_skill3Elapsed = 0.0;
    qreal m_aboutScrollOffset = 0.0;
    int m_heroMoveFrameIndex = 0;
    int m_defeatFrameIndex = -1;
    int m_victoryFrameIndex = -1;
    bool m_dragonSpawned = false;
    bool m_boss2Spawned = false;
    QPixmap m_heroIdlePixmap;
    QPixmap m_heroChangedPixmap;
    QPixmap m_heroBloodPixmap;
    QPixmap m_currentDefeatFrame;
    QPixmap m_currentVictoryFrame;
    QPixmap m_mapPixmap;
    QPixmap m_startMenuPixmap;
    QPixmap m_startButtonPixmap;
    QPixmap m_aboutButtonPixmap;
    QPixmap m_iconButtonPixmap;
    QPixmap m_iconShowPixmap;
    QPixmap m_skill3LaserPixmap;
    QStringList m_defeatFramePaths;
    QStringList m_victoryFramePaths;
};

#endif // MAINWINDOW_H
