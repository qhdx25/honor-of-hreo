#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>// 主窗口基类
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
class MainWindow : public QMainWindow//主窗口类
{
public:
    // 创建主窗口，并初始化资源、定时器和界面控件。
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void initScene();
protected:
    void paintEvent(QPaintEvent *event) override;    //绘制当前帧
    void keyPressEvent(QKeyEvent *event) override;    //记录按下的移动键
    void keyReleaseEvent(QKeyEvent *event) override;//松开按键时，从当前按键集合中移除对应方向键。
    void mousePressEvent(QMouseEvent *event) override;  //处理菜单按钮点击，以及战斗中的普通攻击点击。
    void wheelEvent(QWheelEvent *event) override;//处理关于页面的鼠标滚轮滚动。
    void focusOutEvent(QFocusEvent *event) override;//窗口失去焦点时清空移动状态，避免角色持续移动。
private:    //表示当前正在瞄准或释放的技能类型。
    enum class SkillType {
        None,
        Skill1,
        Skill2,
        Skill3,
        Skill6,
        Skill7,
        Flash
    };
    struct Skill2Explosion {   //二技能爆炸特效运行时数据。
        QPointF center;
        qreal elapsed = 0.0;
    };
    struct BulletWheelBurst {//E键
        QPointF center;
        qreal elapsed = 0.0;
    };
    struct DragonAttackWave {//火龙王攻击时的波纹特效运行时数据。
        QPointF center;
        qreal elapsed = 0.0;
        qreal rotationSeed = 0.0;
        qreal scale = 1.0;
    };
    struct DragonDeathBurst {//龙类 Boss 死亡时的爆炸特效运行时数据
        QPointF center;
        qreal elapsed = 0.0;
    };
    //普通敌人生成函数。
    void spawnEnemy();
    //主游戏循环函数
    void updateBullets();
    // 敌人 AI 更新函数。
    void updateEnemies();
    // 技能冷却和临时特效更新函数。
    void updateSkillCooldowns();
    void updateSkill2Effects();
    void updateBulletWheelEffects();
    void updateDragonEffects();
    //技能瞄准流程：按下技能图标、拖拽瞄准、松手释放。
    void beginSkillAim(SkillType skill);
    void updateSkillAim(SkillType skill, const QPoint &dragOffset);
    void releaseSkill(SkillType skill, const QPoint &dragOffset);
    void clearSkillAim();
    //各个技能的释放函数。
    void castSkill1();
    void castSkill2();
    void castSkill3();
    void applySkill2AreaDamage(const QPointF &impactCenter, const QPointF &impactDirection, int damage);
    void castSkill6();
    void castSkill7();
    void castBulletWheel();
    void castFlash();
    void castTreatment();
    //英雄移动、英雄动画和特殊技能状态更新。
    void updateFlashState();
    void updateHeroMovement();
    void updateHeroAnimation();
    void updateSkill3Effect();
    void playSkillReadySound();
    //游戏状态切换函数。
    void startGame();
    void startDefeatSequence();
    void advanceDefeatFrame();
    void startVictorySequence();
    void advanceVictoryFrame();
    void returnToMainMenu();
    void resetGameplayState();
    void setGameplayUiVisible(bool visible);
    //菜单和关于页面的布局辅助函数。
    QRect startButtonRect() const;
    QRect aboutButtonRect() const;
    QRect iconButtonRect() const;
    QRect aboutBackButtonRect() const;
    QRectF aboutPagePanelRect() const;
    QRectF aboutPageTextViewportRect() const;
    QString aboutPageText() const;
    qreal aboutPageMaxScroll() const;
    //特效和界面 HUD 的绘制辅助函数。
    void drawSkill2Effects(QPainter &painter) const;
    void drawBulletWheelEffects(QPainter &painter) const;
    void drawDragonEffects(QPainter &painter) const;
    void drawSkillArrow(QPainter &painter) const;
    void drawFlashEffect(QPainter &painter) const;
    void drawHeroHealthBar(QPainter &painter) const;
    void drawSkill3Effect(QPainter &painter) const;
    //特殊敌人和 Boss 的生成函数。
    void spawnDragonEnemy();
    void spawnBoss2Enemy();
    void spawnBoss3Enemy();
    //碰撞、暂停、敌人死亡处理和音效辅助函数。
    void resolveEnemyOverlap();
    void togglePause();
    void setPaused(bool paused);
    void handleEnemyDefeat(int enemyIndex);
    void playDragonSpawnSound();
    void playDragonDeathSound();
    void updateMedicinePack();
    //相机和世界坐标辅助函数。
    QPointF cameraOffset() const;
    int worldWidth() const;
    int worldHeight() const;
    QPointF heroCenter() const;
    // 战斗界面上显示的技能图标控件。
    SkillIconWidget *m_skill1Icon = nullptr;
    SkillIconWidget *m_skill2Icon = nullptr;
    SkillIconWidget *m_skill3Icon = nullptr;
    SkillIconWidget *m_skill6Icon = nullptr;
    SkillIconWidget *m_skill7Icon = nullptr;
    SkillIconWidget *m_flashIcon = nullptr;
    SkillIconWidget *m_treatmentIcon = nullptr;
    // 定时器：驱动主循环、敌人刷新和胜负动画。
    QTimer *m_gameTimer = nullptr;
    QTimer *m_enemyTimer = nullptr;
    QTimer *m_defeatFrameTimer = nullptr;
    QTimer *m_victoryFrameTimer = nullptr;
    // 音频播放器：负责菜单音乐、战斗音乐、音效和语音。
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
    //暂停按钮和核心场景对象。
    QPushButton *m_pauseButton = nullptr;
    hero *myHero = nullptr;
    Pet *m_pet = nullptr;
    Crystal *m_crystal = nullptr;
    MedicinePack *m_medicinePack = nullptr;
    //场景对象数组：防御塔、子弹和敌人。
    QVector<Tower *> m_towers;
    QVector<Bullet *> m_bullets;
    QVector<Bullet *> m_enemyBullets;
    QVector<Bullet *> m_petBullets;
    QVector<Enemy *> m_enemies;
    //运行中的特效数组，每个元素代表一个正在播放的特效。
    QVector<Skill2Explosion> m_skill2Explosions;
    QVector<BulletWheelBurst> m_bulletWheelBursts;
    QVector<DragonAttackWave> m_dragonAttackWaves;
    QVector<DragonDeathBurst> m_dragonDeathBursts;
    //当前三技能持续期间已经命中过的敌人，避免重复伤害。
    QVector<Enemy *> m_skill3HitEnemies;
    //英雄移动动画帧。
    QVector<QPixmap> m_heroMoveFrames;
    //当前按下的移动键，由 updateHeroMovement() 使用。
    QSet<int> m_pressedMovementKeys;
   //全局游戏状态标志。
    bool m_gameStarted = false;
    bool m_showAboutPage = false;
    bool m_showIconPage = false;
    bool m_defeatSequenceActive = false;
    bool m_victorySequenceActive = false;
    //当前技能瞄准、暂停和英雄动作状态。
    SkillType m_activeSkill = SkillType::None;
    bool m_skillAiming = false;
    bool m_gamePaused = false;
    bool m_heroMoving = false;
    bool m_heroFacingLeft = false;
    bool m_flashEffectActive = false;
    bool m_skill3Active = false;
    // 移动、闪现和技能瞄准用到的方向与位置数据。
    QPointF m_skillDirection = QPointF(1.0, 0.0);
    QPointF m_heroVelocity;
    QPointF m_flashEffectStartPos;
    QPointF m_flashEffectEndPos;
    QPointF m_skill3BaseDirection = QPointF(1.0, 0.0);
   // 技能冷却剩余时间，单位为毫秒。
    qreal m_skill1CooldownRemainingMs = 0.0;
    qreal m_skill2CooldownRemainingMs = 0.0;
    qreal m_skill3CooldownRemainingMs = 0.0;
    qreal m_skill6CooldownRemainingMs = 0.0;
    qreal m_flashCooldownRemainingMs = 0.0;
    qreal m_treatmentCooldownRemainingMs = 0.0;
   //特效、动画、刷怪等逻辑使用的计时变量。
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
    // 英雄移动动画和胜负动画当前播放到的帧下标。
    int m_heroMoveFrameIndex = 0;
    int m_defeatFrameIndex = -1;
    int m_victoryFrameIndex = -1;
    // Boss 生成标志
    bool m_dragonSpawned = false;
    bool m_boss2Spawned = false;
    // 绘制时使用的图片缓存。
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
    // 胜利和失败动画的帧图片路径。
    QStringList m_defeatFramePaths;
    QStringList m_victoryFramePaths;
};
#endif
