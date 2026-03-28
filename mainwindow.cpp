#include "mainwindow.h"

#include "assetpaths.h"
#include "bullet.h"
#include "bullet3.h"
#include "bullet4.h"
#include "bullet5.h"
#include "bullet6.h"
#include "bullet7.h"
#include "bullet8.h"
#include "bullet9.h"
#include "bullet10.h"
#include "bullet11.h"
#include "bullet12.h"
#include "bullet13.h"
#include "bullet14.h"
#include "bullet15.h"
#include "bullet16.h"
#include "config.h"
#include "crystal.h"
#include "enemy.h"
#include "skill2bullet.h"
#include "skilliconwidget.h"
#include "tower.h"

#include <QDir>
#include <QIcon>
#include <QKeyEvent>
#include <QMediaPlayer>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QRandomGenerator>
#include <QTimer>
#include <QUrl>
#include <QBrush>
#include <QString>
#include <algorithm>
#include <cmath>

namespace {
constexpr qreal kPi = 3.14159265358979323846;
constexpr qreal kHalfPi = kPi / 2.0;
constexpr qreal kSkill3DurationMs = 320.0;
constexpr qreal kSkill3SweepDegrees = 50.0;
constexpr qreal kSkill3Range = 820.0;
constexpr qreal kSkill3VisualWidth = 170.0;
constexpr qreal kSkill3HitWidth = 120.0;
constexpr int kSkill3Damage = 160;
constexpr qreal kSkill1CooldownMs = 2000.0;
constexpr qreal kSkill2CooldownMs = 5000.0;
constexpr qreal kSkill3CooldownMs = 16000.0;
constexpr qreal kSkill2ExplosionDurationMs = 280.0;
constexpr qreal kSkill2ExplosionMaxRadius = 110.0;
constexpr qreal kBulletWheelBurstDurationMs = 360.0;
constexpr qreal kBulletWheelBurstMaxRadius = 170.0;
constexpr qreal kHeroMoveFrameDurationMs = 80.0;
constexpr qreal kHeroMoveHoldDurationMs = 140.0;
constexpr qreal kHeroMoveAcceleration = 0.22;
constexpr qreal kHeroMoveBrake = 0.76;
constexpr qreal kHeroMoveStopThreshold = 0.2;
constexpr int kHeroMoveFrameWidth = HERO_WIDTH + 20;
constexpr int kHeroMoveFrameHeight = HERO_HEIGHT + 20;
constexpr int kHeroHpBarWidth = 170;
constexpr int kHeroHpBarHeight = 26;
constexpr qreal kFlashCooldownMs = 6000.0;
constexpr qreal kFlashDistance = 260.0;
constexpr qreal kFlashEffectDurationMs = 220.0;
constexpr qreal kFlashImpactMaxRadius = 86.0;
constexpr qreal kTreatmentCooldownMs = 20000.0;
constexpr int kTreatmentHealAmount = 220;
constexpr qreal kMapDisplayScale = 0.36;
constexpr int kDefeatAnimationFrameIntervalMs = 67;
constexpr qreal kHeroVoiceInitialDelayMs = 10000.0;
constexpr qreal kHeroVoiceRepeatDelayMs = 120000.0;

QString assetPath(const QString &fileName)
{
    return QString::fromUtf8(kAssetDir) + "/" + fileName;
}

QVector<QPointF> towerPositionsForWorld(int worldWidth, int worldHeight)
{
    return QVector<QPointF>{
        QPointF(worldWidth * 0.49, worldHeight * 0.16),
        QPointF(worldWidth * 0.66, worldHeight * 0.31),
        QPointF(worldWidth * 0.93, worldHeight * 0.46)
    };
}

QPointF normalized(const QPointF &vector)
{
    const qreal length = std::hypot(vector.x(), vector.y());
    if (length <= 0.0001) {
        return QPointF(1.0, 0.0);
    }

    return QPointF(vector.x() / length, vector.y() / length);
}

QPointF rotated(const QPointF &vector, qreal radians)
{
    const qreal cosValue = std::cos(radians);
    const qreal sinValue = std::sin(radians);
    return QPointF(vector.x() * cosValue - vector.y() * sinValue,
                   vector.x() * sinValue + vector.y() * cosValue);
}

qreal distancePointToSegment(const QPointF &point, const QPointF &start, const QPointF &end)
{
    const QPointF segment = end - start;
    const qreal segmentLengthSquared = segment.x() * segment.x() + segment.y() * segment.y();
    if (segmentLengthSquared <= 0.0001) {
        return QLineF(point, start).length();
    }

    const QPointF pointOffset = point - start;
    const qreal projection = std::clamp((pointOffset.x() * segment.x() + pointOffset.y() * segment.y()) / segmentLengthSquared,
                                        0.0,
                                        1.0);
    const QPointF closestPoint = start + segment * projection;
    return QLineF(point, closestPoint).length();
}
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initScene();

    myHero = new hero();

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(16);
    QObject::connect(m_gameTimer, &QTimer::timeout, this, [this]() { updateBullets(); });

    m_menuBgmPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_menuBgmPlayer->setSource(QUrl::fromLocalFile(assetPath("startmusic.mp3")));
#else
    m_menuBgmPlayer->setMedia(QUrl::fromLocalFile(assetPath("startmusic.mp3")));
    m_menuBgmPlayer->setVolume(45);
#endif
    QObject::connect(m_menuBgmPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia || m_menuBgmPlayer == nullptr) {
            return;
        }

        m_menuBgmPlayer->setPosition(0);
        m_menuBgmPlayer->play();
    });
    m_menuBgmPlayer->play();

    m_bgmPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_bgmPlayer->setSource(QUrl::fromLocalFile(assetPath("music.mp3")));
#else
    m_bgmPlayer->setMedia(QUrl::fromLocalFile(assetPath("music.mp3")));
    m_bgmPlayer->setVolume(40);
#endif
    QObject::connect(m_bgmPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia || m_bgmPlayer == nullptr) {
            return;
        }
#if QT_VERSION_MAJOR >= 6
        m_bgmPlayer->setPosition(0);
#else
        m_bgmPlayer->setPosition(0);
#endif
        m_bgmPlayer->play();
    });

    m_defeatAudioPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_defeatAudioPlayer->setSource(QUrl::fromLocalFile(assetPath("defeat_audio.mp3")));
#else
    m_defeatAudioPlayer->setMedia(QUrl::fromLocalFile(assetPath("defeat_audio.mp3")));
    m_defeatAudioPlayer->setVolume(100);
#endif

    m_skill2HitPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_skill2HitPlayer->setSource(QUrl::fromLocalFile(assetPath("secondmusic.mp3")));
#else
    m_skill2HitPlayer->setMedia(QUrl::fromLocalFile(assetPath("secondmusic.mp3")));
    m_skill2HitPlayer->setVolume(90);
#endif

    m_heroVoicePlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_heroVoicePlayer->setSource(QUrl::fromLocalFile(assetPath("heroword.mp3")));
#else
    m_heroVoicePlayer->setMedia(QUrl::fromLocalFile(assetPath("heroword.mp3")));
    m_heroVoicePlayer->setVolume(100);
#endif

    m_skill3VoicePlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_skill3VoicePlayer->setSource(QUrl::fromLocalFile(assetPath("heroword2.mp3")));
#else
    m_skill3VoicePlayer->setMedia(QUrl::fromLocalFile(assetPath("heroword2.mp3")));
    m_skill3VoicePlayer->setVolume(100);
#endif

    m_skillReadyPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_skillReadyPlayer->setSource(QUrl::fromLocalFile(assetPath("ready_music.mp3")));
#else
    m_skillReadyPlayer->setMedia(QUrl::fromLocalFile(assetPath("ready_music.mp3")));
    m_skillReadyPlayer->setVolume(95);
#endif

    const QDir defeatFrameDir(assetPath("defeat_frames"));
    const QStringList defeatFrameNames = defeatFrameDir.entryList(QStringList() << "frame_*.jpg",
                                                                 QDir::Files,
                                                                 QDir::Name);
    for (const QString &frameName : defeatFrameNames) {
        m_defeatFramePaths.push_back(defeatFrameDir.absoluteFilePath(frameName));
    }

    m_defeatFrameTimer = new QTimer(this);
    m_defeatFrameTimer->setInterval(kDefeatAnimationFrameIntervalMs);
    QObject::connect(m_defeatFrameTimer, &QTimer::timeout, this, [this]() { advanceDefeatFrame(); });

    m_victoryAudioPlayer = new QMediaPlayer(this);
#if QT_VERSION_MAJOR >= 6
    m_victoryAudioPlayer->setSource(QUrl::fromLocalFile(assetPath("victory_audio.mp3")));
#else
    m_victoryAudioPlayer->setMedia(QUrl::fromLocalFile(assetPath("victory_audio.mp3")));
    m_victoryAudioPlayer->setVolume(100);
#endif

    const QDir victoryFrameDir(assetPath("victory_frames"));
    const QStringList victoryFrameNames = victoryFrameDir.entryList(QStringList() << "frame_*.jpg",
                                                                   QDir::Files,
                                                                   QDir::Name);
    for (const QString &frameName : victoryFrameNames) {
        m_victoryFramePaths.push_back(victoryFrameDir.absoluteFilePath(frameName));
    }

    m_victoryFrameTimer = new QTimer(this);
    m_victoryFrameTimer->setInterval(kDefeatAnimationFrameIntervalMs);
    QObject::connect(m_victoryFrameTimer, &QTimer::timeout, this, [this]() { advanceVictoryFrame(); });

    m_enemyTimer = new QTimer(this);
    m_enemyTimer->setInterval(1800);
    QObject::connect(m_enemyTimer, &QTimer::timeout, this, [this]() { spawnEnemy(); });

    m_startMenuPixmap.load(assetPath("firstmenu.jpg"));
    m_startButtonPixmap.load(assetPath("startbutton.png"));
    m_heroIdlePixmap.load(assetPath("unmove.png"));
    m_heroBloodPixmap.load(assetPath("blood.png"));
    m_heroMoveFrames = QVector<QPixmap>{
        QPixmap(assetPath("run1.png")),
        QPixmap(assetPath("run2.png")),
        QPixmap(assetPath("run3.png")),
        QPixmap(assetPath("run4.png")),
        QPixmap(assetPath("run5.png")),
        QPixmap(assetPath("run6.png")),
        QPixmap(assetPath("run7.png")),
        QPixmap(assetPath("run8.png")),
        QPixmap(assetPath("run9.png")),
        QPixmap(assetPath("run10.png"))
    };
    const QPixmap rawMapPixmap(assetPath("hok_gorge_playfield_final_clean.png"));
    if (!rawMapPixmap.isNull()) {
        m_mapPixmap = rawMapPixmap.scaled(static_cast<int>(rawMapPixmap.width() * kMapDisplayScale),
                                          static_cast<int>(rawMapPixmap.height() * kMapDisplayScale),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
    }
    m_skill3LaserPixmap.load(assetPath("angela_skill3.png"));

    if (!m_mapPixmap.isNull()) {
        myHero->setPosition(720, worldHeight() - 900);
    }

    m_crystal = new Crystal();
    m_crystal->setCenter(QPointF(worldWidth() - 220.0, 190.0));
    for (int i = 0; i < 3; ++i) {
        m_towers.push_back(new Tower());
    }
    const QVector<QPointF> towerPositions = towerPositionsForWorld(worldWidth(), worldHeight());
    for (int i = 0; i < m_towers.size() && i < towerPositions.size(); ++i) {
        m_towers.at(i)->setCenter(towerPositions.at(i));
    }

    const QPoint skill1Pos(1675, 920);
    m_skill1Icon = new SkillIconWidget(this);
    m_skill1Icon->setFocusPolicy(Qt::NoFocus);
    m_skill1Icon->setFrames(QVector<QString>{assetPath("2.png")});
    m_skill1Icon->setGeometry(skill1Pos.x(),
                              skill1Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
    m_skill1Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill1);
    });
    m_skill1Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill1, dragOffset);
    });
    m_skill1Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill1, dragOffset);
    });
    m_skill1Icon->setCooldownState(0.0, kSkill1CooldownMs);

    const QPoint skill2Pos(1755, 805);
    m_skill2Icon = new SkillIconWidget(this);
    m_skill2Icon->setFocusPolicy(Qt::NoFocus);
    m_skill2Icon->setFrames(QVector<QString>{assetPath("3.png")});
    m_skill2Icon->setGeometry(skill2Pos.x(),
                              skill2Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
    m_skill2Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill2);
    });
    m_skill2Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill2, dragOffset);
    });
    m_skill2Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill2, dragOffset);
    });
    m_skill2Icon->setCooldownState(0.0, kSkill2CooldownMs);

    const QPoint skill3Pos(1835, 690);
    m_skill3Icon = new SkillIconWidget(this);
    m_skill3Icon->setFocusPolicy(Qt::NoFocus);
    m_skill3Icon->setFrames(QVector<QString>{assetPath("skill3.png")});
    m_skill3Icon->setGeometry(skill3Pos.x(),
                              skill3Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
    m_skill3Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill3);
    });
    m_skill3Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill3, dragOffset);
    });
    m_skill3Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill3, dragOffset);
    });
    m_skill3Icon->setCooldownState(0.0, kSkill3CooldownMs);

    const QPoint treatmentPos(1545, 955);
    m_treatmentIcon = new SkillIconWidget(this);
    m_treatmentIcon->setFocusPolicy(Qt::NoFocus);
    m_treatmentIcon->setFrames(QVector<QString>{assetPath("treatment.png")});
    m_treatmentIcon->setGeometry(treatmentPos.x(),
                                 treatmentPos.y(),
                                 SkillIconWidget::IconSize,
                                 SkillIconWidget::IconSize);
    m_treatmentIcon->setClickHandler([this]() {
        castTreatment();
        update();
    });
    m_treatmentIcon->setCooldownState(0.0, kTreatmentCooldownMs);

    const QPoint flashPos(1415, 965);
    m_flashIcon = new SkillIconWidget(this);
    m_flashIcon->setFocusPolicy(Qt::NoFocus);
    m_flashIcon->setFrames(QVector<QString>{assetPath("fastmove.png")});
    m_flashIcon->setGeometry(flashPos.x(),
                             flashPos.y(),
                             SkillIconWidget::IconSize,
                             SkillIconWidget::IconSize);
    m_flashIcon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Flash);
    });
    m_flashIcon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Flash, dragOffset);
    });
    m_flashIcon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Flash, dragOffset);
    });
    m_flashIcon->setCooldownState(0.0, kFlashCooldownMs);

    setGameplayUiVisible(false);
}

MainWindow::~MainWindow()
{
    for (Bullet *bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();

    for (Bullet *bullet : m_enemyBullets) {
        delete bullet;
    }
    m_enemyBullets.clear();

    for (Enemy *enemy : m_enemies) {
        delete enemy;
    }
    m_enemies.clear();

    for (Tower *tower : m_towers) {
        delete tower;
    }
    m_towers.clear();

    delete m_crystal;
    delete myHero;
}

void MainWindow::initScene()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle(GAME_TITLE);
    setWindowIcon(QIcon(assetPath("firsta.jpg")));
    setFocusPolicy(Qt::StrongFocus);
    setFocus();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (m_defeatSequenceActive) {
        if (!m_currentDefeatFrame.isNull()) {
            painter.drawPixmap(rect(), m_currentDefeatFrame);
        } else {
            painter.fillRect(rect(), Qt::black);
        }
        return;
    }

    if (m_victorySequenceActive) {
        if (!m_currentVictoryFrame.isNull()) {
            painter.drawPixmap(rect(), m_currentVictoryFrame);
        } else {
            painter.fillRect(rect(), Qt::black);
        }
        return;
    }

    if (!m_gameStarted) {
        if (!m_startMenuPixmap.isNull()) {
            painter.drawPixmap(rect(), m_startMenuPixmap);
        } else {
            painter.fillRect(rect(), QColor(18, 21, 27));
        }

        const QRect buttonRect = startButtonRect();
        if (!m_startButtonPixmap.isNull()) {
            painter.drawPixmap(buttonRect, m_startButtonPixmap);
        } else {
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(239, 179, 72));
            painter.drawRoundedRect(buttonRect, 18, 18);
            painter.setPen(QColor(58, 30, 8));
            painter.drawText(buttonRect, Qt::AlignCenter, QStringLiteral("开始游戏"));
        }
        return;
    }

    const QPointF camera = cameraOffset();

    if (!m_mapPixmap.isNull()) {
        painter.drawPixmap(QRectF(-camera.x(),
                                  -camera.y(),
                                  m_mapPixmap.width(),
                                  m_mapPixmap.height()),
                           m_mapPixmap,
                           QRectF(0.0, 0.0, m_mapPixmap.width(), m_mapPixmap.height()));
    } else {
        QPixmap bgPix(assetPath("background2.jpg"));
        painter.drawPixmap(0, 0, width(), height(), bgPix);
    }

    painter.save();
    painter.translate(-camera);

    if (m_flashEffectActive) {
        drawFlashEffect(painter);
    }

    if (myHero != nullptr) {
        const bool useMoveFrame =
            m_heroMoving && !m_heroMoveFrames.isEmpty() && !m_heroMoveFrames.at(m_heroMoveFrameIndex).isNull();
        const QPixmap &heroPix = useMoveFrame ? m_heroMoveFrames.at(m_heroMoveFrameIndex) : m_heroIdlePixmap;
        const int drawWidth = useMoveFrame ? kHeroMoveFrameWidth : HERO_WIDTH;
        const int drawHeight = useMoveFrame ? kHeroMoveFrameHeight : HERO_HEIGHT;
        const int drawX = myHero->Hero_x - (drawWidth - HERO_WIDTH) / 2;
        const int drawY = myHero->Hero_y - (drawHeight - HERO_HEIGHT) / 2;
        if (m_heroFacingLeft && !heroPix.isNull()) {
            painter.save();
            painter.translate(drawX + drawWidth, drawY);
            painter.scale(-1.0, 1.0);
            painter.drawPixmap(0, 0, drawWidth, drawHeight, heroPix);
            painter.restore();
        } else {
            painter.drawPixmap(drawX, drawY, drawWidth, drawHeight, heroPix);
        }
        drawHeroHealthBar(painter);
    }

    if (m_skillAiming) {
        drawSkillArrow(painter);
    }

    for (const Bullet *bullet : m_bullets) {
        bullet->paint(painter);
    }

    for (const Bullet *bullet : m_enemyBullets) {
        bullet->paint(painter);
    }

    for (const Enemy *enemy : m_enemies) {
        enemy->paint(painter);
    }

    for (const Tower *tower : m_towers) {
        tower->paint(painter);
    }

    if (m_crystal != nullptr) {
        m_crystal->paint(painter);
    }

    if (!m_skill2Explosions.isEmpty()) {
        drawSkill2Effects(painter);
    }

    if (!m_bulletWheelBursts.isEmpty()) {
        drawBulletWheelEffects(painter);
    }

    if (m_skill3Active) {
        drawSkill3Effect(painter);
    }

    painter.restore();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_A:
    case Qt::Key_S:
    case Qt::Key_D:
        m_pressedMovementKeys.insert(event->key());
        break;
    case Qt::Key_E:
        if (!event->isAutoRepeat()) {
            castBulletWheel();
        }
        break;
    default:
        break;
    }

    QMainWindow::keyPressEvent(event);
    update();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        QMainWindow::keyReleaseEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_A:
    case Qt::Key_S:
    case Qt::Key_D:
        m_pressedMovementKeys.remove(event->key());
        break;
    default:
        break;
    }

    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (!m_gameStarted) {
        if (event->button() == Qt::LeftButton && startButtonRect().contains(event->pos())) {
            startGame();
            update();
            return;
        }

        QMainWindow::mousePressEvent(event);
        return;
    }

    if (m_defeatSequenceActive || m_victorySequenceActive) {
        QMainWindow::mousePressEvent(event);
        return;
    }

    if (event->button() == Qt::LeftButton && myHero != nullptr) {
        m_bullets.push_back(new Bullet(myHero->shootOrigin(), QPointF(event->pos()) + cameraOffset()));
        update();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    m_pressedMovementKeys.clear();
    m_heroVelocity = QPointF(0.0, 0.0);
    m_heroMoveHoldElapsed = 0.0;
    m_heroMoving = false;

    QMainWindow::focusOutEvent(event);
}

void MainWindow::startGame()
{
    if (m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    resetGameplayState();
    m_gameStarted = true;
    setGameplayUiVisible(true);
    if (m_menuBgmPlayer != nullptr) {
        m_menuBgmPlayer->stop();
    }
    if (m_gameTimer != nullptr && !m_gameTimer->isActive()) {
        m_gameTimer->start();
    }
    if (m_enemyTimer != nullptr && !m_enemyTimer->isActive()) {
        m_enemyTimer->start();
    }
    if (m_bgmPlayer != nullptr) {
        m_bgmPlayer->setPosition(0);
        m_bgmPlayer->play();
    }
    m_heroVoiceCountdownMs = kHeroVoiceInitialDelayMs;
    setFocus();
}

void MainWindow::startDefeatSequence()
{
    if (!m_gameStarted || m_defeatSequenceActive) {
        return;
    }

    m_defeatSequenceActive = true;
    clearSkillAim();
    setGameplayUiVisible(false);

    if (m_gameTimer != nullptr) {
        m_gameTimer->stop();
    }
    if (m_enemyTimer != nullptr) {
        m_enemyTimer->stop();
    }
    if (m_bgmPlayer != nullptr) {
        m_bgmPlayer->stop();
    }
    if (m_defeatAudioPlayer != nullptr) {
        m_defeatAudioPlayer->setPosition(0);
        m_defeatAudioPlayer->play();
    }

    if (m_defeatFramePaths.isEmpty()) {
        returnToMainMenu();
        return;
    }

    m_defeatFrameIndex = 0;
    m_currentDefeatFrame.load(m_defeatFramePaths.at(m_defeatFrameIndex));
    update();

    if (m_defeatFrameTimer != nullptr && !m_defeatFrameTimer->isActive()) {
        m_defeatFrameTimer->start();
    }
}

void MainWindow::advanceDefeatFrame()
{
    if (!m_defeatSequenceActive) {
        return;
    }

    ++m_defeatFrameIndex;
    if (m_defeatFrameIndex >= m_defeatFramePaths.size()) {
        if (m_defeatFrameTimer != nullptr) {
            m_defeatFrameTimer->stop();
        }
        returnToMainMenu();
        return;
    }

    m_currentDefeatFrame.load(m_defeatFramePaths.at(m_defeatFrameIndex));
    update();
}

void MainWindow::startVictorySequence()
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    m_victorySequenceActive = true;
    clearSkillAim();
    setGameplayUiVisible(false);

    if (m_gameTimer != nullptr) {
        m_gameTimer->stop();
    }
    if (m_enemyTimer != nullptr) {
        m_enemyTimer->stop();
    }
    if (m_bgmPlayer != nullptr) {
        m_bgmPlayer->stop();
    }
    if (m_victoryAudioPlayer != nullptr) {
        m_victoryAudioPlayer->setPosition(0);
        m_victoryAudioPlayer->play();
    }

    if (m_victoryFramePaths.isEmpty()) {
        returnToMainMenu();
        return;
    }

    m_victoryFrameIndex = 0;
    m_currentVictoryFrame.load(m_victoryFramePaths.at(m_victoryFrameIndex));
    update();

    if (m_victoryFrameTimer != nullptr && !m_victoryFrameTimer->isActive()) {
        m_victoryFrameTimer->start();
    }
}

void MainWindow::advanceVictoryFrame()
{
    if (!m_victorySequenceActive) {
        return;
    }

    ++m_victoryFrameIndex;
    if (m_victoryFrameIndex >= m_victoryFramePaths.size()) {
        if (m_victoryFrameTimer != nullptr) {
            m_victoryFrameTimer->stop();
        }
        returnToMainMenu();
        return;
    }

    m_currentVictoryFrame.load(m_victoryFramePaths.at(m_victoryFrameIndex));
    update();
}

void MainWindow::returnToMainMenu()
{
    if (m_defeatFrameTimer != nullptr) {
        m_defeatFrameTimer->stop();
    }
    if (m_victoryFrameTimer != nullptr) {
        m_victoryFrameTimer->stop();
    }
    if (m_defeatAudioPlayer != nullptr) {
        m_defeatAudioPlayer->stop();
    }
    if (m_victoryAudioPlayer != nullptr) {
        m_victoryAudioPlayer->stop();
    }
    if (m_skill2HitPlayer != nullptr) {
        m_skill2HitPlayer->stop();
    }
    if (m_heroVoicePlayer != nullptr) {
        m_heroVoicePlayer->stop();
    }
    if (m_skill3VoicePlayer != nullptr) {
        m_skill3VoicePlayer->stop();
    }
    if (m_skillReadyPlayer != nullptr) {
        m_skillReadyPlayer->stop();
    }

    resetGameplayState();
    m_gameStarted = false;
    m_defeatSequenceActive = false;
    m_victorySequenceActive = false;
    setGameplayUiVisible(false);

    if (m_menuBgmPlayer != nullptr) {
        m_menuBgmPlayer->setPosition(0);
        m_menuBgmPlayer->play();
    }
    update();
}

void MainWindow::resetGameplayState()
{
    if (m_gameTimer != nullptr) {
        m_gameTimer->stop();
    }
    if (m_enemyTimer != nullptr) {
        m_enemyTimer->stop();
    }
    if (m_defeatFrameTimer != nullptr) {
        m_defeatFrameTimer->stop();
    }
    if (m_victoryFrameTimer != nullptr) {
        m_victoryFrameTimer->stop();
    }
    if (m_defeatAudioPlayer != nullptr) {
        m_defeatAudioPlayer->stop();
    }
    if (m_victoryAudioPlayer != nullptr) {
        m_victoryAudioPlayer->stop();
    }
    if (m_skill2HitPlayer != nullptr) {
        m_skill2HitPlayer->stop();
    }
    if (m_heroVoicePlayer != nullptr) {
        m_heroVoicePlayer->stop();
    }
    if (m_skill3VoicePlayer != nullptr) {
        m_skill3VoicePlayer->stop();
    }
    if (m_skillReadyPlayer != nullptr) {
        m_skillReadyPlayer->stop();
    }

    for (Bullet *bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();

    for (Bullet *bullet : m_enemyBullets) {
        delete bullet;
    }
    m_enemyBullets.clear();

    for (Enemy *enemy : m_enemies) {
        delete enemy;
    }
    m_enemies.clear();

    for (Tower *tower : m_towers) {
        tower->reset();
    }

    m_skill2Explosions.clear();
    m_bulletWheelBursts.clear();
    m_skill3HitEnemies.clear();
    m_pressedMovementKeys.clear();
    m_activeSkill = SkillType::None;
    m_skillAiming = false;
    m_heroMoving = false;
    m_flashEffectActive = false;
    m_skill3Active = false;
    m_skillDirection = QPointF(1.0, 0.0);
    m_heroVelocity = QPointF(0.0, 0.0);
    m_flashEffectStartPos = QPointF();
    m_flashEffectEndPos = QPointF();
    m_skill3BaseDirection = QPointF(1.0, 0.0);
    m_skill1CooldownRemainingMs = 0.0;
    m_skill2CooldownRemainingMs = 0.0;
    m_skill3CooldownRemainingMs = 0.0;
    m_flashCooldownRemainingMs = 0.0;
    m_treatmentCooldownRemainingMs = 0.0;
    m_flashEffectElapsed = 0.0;
    m_skillDragLength = 0.0;
    m_heroMoveAnimationElapsed = 0.0;
    m_heroMoveHoldElapsed = 0.0;
    m_heroVoiceCountdownMs = -1.0;
    m_skill3Elapsed = 0.0;
    m_heroMoveFrameIndex = 0;
    m_defeatFrameIndex = -1;
    m_victoryFrameIndex = -1;
    m_currentDefeatFrame = QPixmap();
    m_currentVictoryFrame = QPixmap();

    if (myHero != nullptr) {
        myHero->heal(myHero->maxHp());
        if (!m_mapPixmap.isNull()) {
            myHero->setPosition(720, worldHeight() - 900);
        } else {
            myHero->setPosition(100, 100);
        }
    }
    if (m_crystal != nullptr) {
        m_crystal->reset();
        m_crystal->setCenter(QPointF(worldWidth() - 220.0, 190.0));
    }
    const QVector<QPointF> towerPositions = towerPositionsForWorld(worldWidth(), worldHeight());
    for (int i = 0; i < m_towers.size() && i < towerPositions.size(); ++i) {
        m_towers.at(i)->reset();
        m_towers.at(i)->setCenter(towerPositions.at(i));
    }

    updateSkillCooldowns();
    updateSkill2Effects();
    updateBulletWheelEffects();
    updateFlashState();
}

void MainWindow::setGameplayUiVisible(bool visible)
{
    if (m_skill1Icon != nullptr) {
        m_skill1Icon->setVisible(visible);
    }
    if (m_skill2Icon != nullptr) {
        m_skill2Icon->setVisible(visible);
    }
    if (m_skill3Icon != nullptr) {
        m_skill3Icon->setVisible(visible);
    }
    if (m_flashIcon != nullptr) {
        m_flashIcon->setVisible(visible);
    }
    if (m_treatmentIcon != nullptr) {
        m_treatmentIcon->setVisible(visible);
    }
}

QRect MainWindow::startButtonRect() const
{
    const QSize buttonSize = m_startButtonPixmap.isNull()
                                 ? QSize(320, 120)
                                 : m_startButtonPixmap.size().scaled(360, 160, Qt::KeepAspectRatio);
    return QRect((width() - buttonSize.width()) / 2,
                 (height() - buttonSize.height()) / 2,
                 buttonSize.width(),
                 buttonSize.height());
}

void MainWindow::updateBullets()
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    updateHeroMovement();
    updateSkillCooldowns();
    updateSkill2Effects();
    updateBulletWheelEffects();
    updateFlashState();

    if (m_heroVoiceCountdownMs > 0.0) {
        m_heroVoiceCountdownMs -= m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
        if (m_heroVoiceCountdownMs <= 0.0) {
            if (m_heroVoicePlayer != nullptr) {
                m_heroVoicePlayer->stop();
                m_heroVoicePlayer->setPosition(0);
                m_heroVoicePlayer->play();
            }
            m_heroVoiceCountdownMs += kHeroVoiceRepeatDelayMs;
        }
    }

    if (m_crystal != nullptr && myHero != nullptr) {
        const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
        if (m_crystal->tryShootAt(heroCenter(), deltaMs)) {
            m_enemyBullets.push_back(new Bullet3(m_crystal->shootOrigin(), heroCenter(), 14.0, 1800.0));
        }
    }
    if (myHero != nullptr) {
        const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
        for (Tower *tower : m_towers) {
            if (tower != nullptr && tower->tryShootAt(heroCenter(), deltaMs)) {
                m_enemyBullets.push_back(new Bullet4(tower->shootOrigin(), heroCenter(), 16.0, 1500.0));
            }
        }
    }

    for (int i = m_bullets.size() - 1; i >= 0; --i) {
        Bullet *bullet = m_bullets.at(i);
        bullet->update();

        if (bullet->isOutOfBounds(worldWidth(), worldHeight()) || bullet->hasReachedMaxDistance()) {
            delete bullet;
            m_bullets.removeAt(i);
            continue;
        }

        const QRectF bulletRect = bullet->boundingRect();
        bool bulletConsumed = false;

        if (m_crystal != nullptr && !m_crystal->isDead() && bulletRect.intersects(m_crystal->boundingRect())) {
            m_crystal->takeDamage(bullet->damage());
            delete bullet;
            m_bullets.removeAt(i);
            if (m_crystal->isDead()) {
                startVictorySequence();
                return;
            }
            continue;
        }

        bool hitTower = false;
        for (Tower *tower : m_towers) {
            if (tower == nullptr || tower->isDead() || !bulletRect.intersects(tower->boundingRect())) {
                continue;
            }

            tower->takeDamage(bullet->damage());
            delete bullet;
            m_bullets.removeAt(i);
            hitTower = true;
            break;
        }
        if (hitTower) {
            continue;
        }

        for (int e = m_enemies.size() - 1; e >= 0; --e) {
            Enemy *enemy = m_enemies.at(e);
            if (!bulletRect.intersects(enemy->boundingRect())) {
                continue;
            }

            const bool isSkill2Hit = dynamic_cast<Skill2Bullet *>(bullet) != nullptr;
            enemy->takeDamage(bullet->damage());
            if (isSkill2Hit) {
                Skill2Explosion explosion;
                explosion.center = bulletRect.center();
                m_skill2Explosions.push_back(explosion);
                if (m_skill2HitPlayer != nullptr) {
                    m_skill2HitPlayer->stop();
                    m_skill2HitPlayer->setPosition(0);
                    m_skill2HitPlayer->play();
                }
            }
            delete bullet;
            m_bullets.removeAt(i);
            bulletConsumed = true;

            if (enemy->isDead()) {
                delete enemy;
                m_enemies.removeAt(e);
            }
            break;
        }

        if (bulletConsumed) {
            continue;
        }
    }

    if (myHero != nullptr) {
        const QRectF heroRect(myHero->Hero_x, myHero->Hero_y, HERO_WIDTH, HERO_HEIGHT);
        for (int i = m_enemyBullets.size() - 1; i >= 0; --i) {
            Bullet *bullet = m_enemyBullets.at(i);
            bullet->update();

            if (bullet->isOutOfBounds(worldWidth(), worldHeight()) || bullet->hasReachedMaxDistance()) {
                delete bullet;
                m_enemyBullets.removeAt(i);
                continue;
            }

            if (!bullet->boundingRect().intersects(heroRect)) {
                continue;
            }

            myHero->takeDamage(bullet->damage());
            delete bullet;
            m_enemyBullets.removeAt(i);
        }
    }

    updateHeroAnimation();
    updateSkill3Effect();
    updateEnemies();
    if (myHero != nullptr && myHero->hp() <= 0) {
        startDefeatSequence();
        return;
    }
    update();
}

void MainWindow::updateSkillCooldowns()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    const qreal previousSkill1CooldownMs = m_skill1CooldownRemainingMs;
    const qreal previousSkill3CooldownMs = m_skill3CooldownRemainingMs;
    const qreal previousTreatmentCooldownMs = m_treatmentCooldownRemainingMs;

    if (m_skill1CooldownRemainingMs > 0.0) {
        m_skill1CooldownRemainingMs = std::max(0.0, m_skill1CooldownRemainingMs - deltaMs);
    }
    if (m_skill3CooldownRemainingMs > 0.0) {
        m_skill3CooldownRemainingMs = std::max(0.0, m_skill3CooldownRemainingMs - deltaMs);
    }
    if (m_treatmentCooldownRemainingMs > 0.0) {
        m_treatmentCooldownRemainingMs = std::max(0.0, m_treatmentCooldownRemainingMs - deltaMs);
    }

    if (m_skill1Icon != nullptr) {
        m_skill1Icon->setCooldownState(m_skill1CooldownRemainingMs, kSkill1CooldownMs);
        m_skill1Icon->setEnabled(m_skill1CooldownRemainingMs <= 0.0);
    }
    if (m_skill3Icon != nullptr) {
        m_skill3Icon->setCooldownState(m_skill3CooldownRemainingMs, kSkill3CooldownMs);
        m_skill3Icon->setEnabled(m_skill3CooldownRemainingMs <= 0.0);
    }
    if (m_treatmentIcon != nullptr) {
        m_treatmentIcon->setCooldownState(m_treatmentCooldownRemainingMs, kTreatmentCooldownMs);
        m_treatmentIcon->setEnabled(m_treatmentCooldownRemainingMs <= 0.0);
    }

    if ((previousSkill1CooldownMs > 0.0 && m_skill1CooldownRemainingMs <= 0.0)
        || (previousSkill3CooldownMs > 0.0 && m_skill3CooldownRemainingMs <= 0.0)
        || (previousTreatmentCooldownMs > 0.0 && m_treatmentCooldownRemainingMs <= 0.0)) {
        playSkillReadySound();
    }
}

void MainWindow::updateSkill2Effects()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    const qreal previousSkill2CooldownMs = m_skill2CooldownRemainingMs;

    if (m_skill2CooldownRemainingMs > 0.0) {
        m_skill2CooldownRemainingMs = std::max(0.0, m_skill2CooldownRemainingMs - deltaMs);
    }

    if (m_skill2Icon != nullptr) {
        m_skill2Icon->setCooldownState(m_skill2CooldownRemainingMs, kSkill2CooldownMs);
        m_skill2Icon->setEnabled(m_skill2CooldownRemainingMs <= 0.0);
    }

    if (previousSkill2CooldownMs > 0.0 && m_skill2CooldownRemainingMs <= 0.0) {
        playSkillReadySound();
    }

    for (int i = m_skill2Explosions.size() - 1; i >= 0; --i) {
        Skill2Explosion &explosion = m_skill2Explosions[i];
        explosion.elapsed += deltaMs;
        if (explosion.elapsed < kSkill2ExplosionDurationMs) {
            continue;
        }

        m_skill2Explosions.removeAt(i);
    }
}

void MainWindow::updateFlashState()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    const qreal previousFlashCooldownMs = m_flashCooldownRemainingMs;

    if (m_flashCooldownRemainingMs > 0.0) {
        m_flashCooldownRemainingMs = std::max(0.0, m_flashCooldownRemainingMs - deltaMs);
    }

    if (m_flashIcon != nullptr) {
        m_flashIcon->setCooldownState(m_flashCooldownRemainingMs, kFlashCooldownMs);
        m_flashIcon->setEnabled(m_flashCooldownRemainingMs <= 0.0);
    }

    if (previousFlashCooldownMs > 0.0 && m_flashCooldownRemainingMs <= 0.0) {
        playSkillReadySound();
    }

    if (!m_flashEffectActive) {
        return;
    }

    m_flashEffectElapsed += deltaMs;
    if (m_flashEffectElapsed >= kFlashEffectDurationMs) {
        m_flashEffectActive = false;
        m_flashEffectElapsed = 0.0;
    }
}

void MainWindow::updateHeroMovement()
{
    if (myHero == nullptr) {
        return;
    }

    QPointF inputDirection(0.0, 0.0);
    if (m_pressedMovementKeys.contains(Qt::Key_A)) {
        inputDirection.rx() -= 1.0;
    }
    if (m_pressedMovementKeys.contains(Qt::Key_D)) {
        inputDirection.rx() += 1.0;
    }
    if (m_pressedMovementKeys.contains(Qt::Key_W)) {
        inputDirection.ry() -= 1.0;
    }
    if (m_pressedMovementKeys.contains(Qt::Key_S)) {
        inputDirection.ry() += 1.0;
    }

    const qreal inputLength = std::hypot(inputDirection.x(), inputDirection.y());
    if (inputLength > 0.0001) {
        if (inputDirection.x() < -0.1) {
            m_heroFacingLeft = true;
        } else if (inputDirection.x() > 0.1) {
            m_heroFacingLeft = false;
        }

        const QPointF targetVelocity =
            QPointF(inputDirection.x() / inputLength, inputDirection.y() / inputLength) * myHero->Hero_speed;
        m_heroVelocity = m_heroVelocity * (1.0 - kHeroMoveAcceleration) + targetVelocity * kHeroMoveAcceleration;
    } else {
        m_heroVelocity *= kHeroMoveBrake;
        if (std::hypot(m_heroVelocity.x(), m_heroVelocity.y()) < kHeroMoveStopThreshold) {
            m_heroVelocity = QPointF(0.0, 0.0);
        }
    }

    const QPoint oldPos(myHero->Hero_x, myHero->Hero_y);
    myHero->updatePos(m_heroVelocity, worldWidth(), worldHeight());
    if (oldPos != QPoint(myHero->Hero_x, myHero->Hero_y)) {
        m_heroMoving = true;
        m_heroMoveHoldElapsed = kHeroMoveHoldDurationMs;
    }
}

void MainWindow::spawnEnemy()
{
    if (myHero == nullptr) {
        return;
    }

    const int typeIndex = static_cast<int>(QRandomGenerator::global()->bounded(5u));
    const Enemy::Type type = static_cast<Enemy::Type>(typeIndex);
    const QVector<QPoint> laneSpawnPoints{
        QPoint(worldWidth() - 920, 520),
        QPoint(worldWidth() - 820, worldHeight() / 2 - 180),
        QPoint(worldWidth() - 700, worldHeight() - 1180)
    };
    const QPoint lanePoint = laneSpawnPoints.at(static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(laneSpawnPoints.size()))));
    const int spawnX = std::clamp(lanePoint.x() + static_cast<int>(QRandomGenerator::global()->bounded(120u)) - 60,
                                  0,
                                  worldWidth() - 20);
    const int spawnY = std::clamp(lanePoint.y() + static_cast<int>(QRandomGenerator::global()->bounded(120u)) - 60,
                                  0,
                                  worldHeight() - 20);

    m_enemies.push_back(new Enemy(type, QPointF(spawnX, spawnY)));
}

void MainWindow::updateEnemies()
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF center = heroCenter();
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy *enemy = m_enemies.at(i);
        if (!enemy->reachesTarget(center)) {
            enemy->updateToward(center);
        } else if (enemy->tryAttackTarget(center, deltaMs)) {
            myHero->takeDamage(enemy->attackDamage());
        }
        enemy->updateEnteredState(worldWidth(), worldHeight());

        const bool shouldRemove =
            enemy->hasEnteredScreen() && enemy->isOutOfBounds(worldWidth(), worldHeight());

        if (shouldRemove) {
            delete enemy;
            m_enemies.removeAt(i);
        }
    }
}

void MainWindow::beginSkillAim(SkillType skill)
{
    m_activeSkill = skill;
    m_skillAiming = false;
    m_skillDragLength = 0.0;
    update();
}

void MainWindow::updateSkillAim(SkillType skill, const QPoint &dragOffset)
{
    m_activeSkill = skill;

    const QPointF dragVector = QPointF(dragOffset);
    const qreal length = std::hypot(dragVector.x(), dragVector.y());
    if (length < 12.0) {
        m_skillAiming = false;
        m_skillDragLength = 0.0;
        update();
        return;
    }

    m_skillAiming = true;
    m_skillDirection = normalized(dragVector);
    m_skillDragLength = length;
    update();
}

void MainWindow::releaseSkill(SkillType skill, const QPoint &dragOffset)
{
    updateSkillAim(skill, dragOffset);

    if (myHero == nullptr || !m_skillAiming) {
        clearSkillAim();
        return;
    }

    switch (skill) {
    case SkillType::Skill1:
        castSkill1();
        break;
    case SkillType::Skill2:
        castSkill2();
        break;
    case SkillType::Skill3:
        castSkill3();
        break;
    case SkillType::Flash:
        castFlash();
        break;
    case SkillType::None:
        break;
    }

    clearSkillAim();
    update();
}

void MainWindow::castSkill1()
{
    if (myHero == nullptr || m_skill1CooldownRemainingMs > 0.0) {
        return;
    }

    const QPointF origin = myHero->shootOrigin();
    constexpr int bulletCount = 6;
    constexpr qreal spreadDegrees = 60.0;
    const qreal stepDegrees = spreadDegrees / (bulletCount - 1);

    for (int i = 0; i < bulletCount; ++i) {
        const qreal degrees = -spreadDegrees / 2.0 + i * stepDegrees;
        const qreal radians = degrees * kPi / 180.0;
        const QPointF direction = rotated(m_skillDirection, radians);
        const QPointF target = origin + direction * 300.0;
        m_bullets.push_back(new Bullet(origin, target, 24.0, 960.0, QSize(64, 64)));
    }

    m_skill1CooldownRemainingMs = kSkill1CooldownMs;
}

void MainWindow::castSkill2()
{
    if (myHero == nullptr || m_skill2CooldownRemainingMs > 0.0) {
        return;
    }

    const QPointF origin = myHero->shootOrigin();
    const QPointF target = origin + m_skillDirection * 480.0;
    m_bullets.push_back(new Skill2Bullet(origin, target, 20.0, 1200.0));
    m_skill2CooldownRemainingMs = kSkill2CooldownMs;
}

void MainWindow::castSkill3()
{
    if (myHero == nullptr || m_skill3CooldownRemainingMs > 0.0) {
        return;
    }

    m_skill3Active = true;
    m_skill3Elapsed = 0.0;
    m_skill3BaseDirection = normalized(m_skillDirection);
    m_skill3HitEnemies.clear();
    m_skill3CooldownRemainingMs = kSkill3CooldownMs;

    bool heroVoicePlaying = false;
#if QT_VERSION_MAJOR >= 6
    heroVoicePlaying = m_heroVoicePlayer != nullptr
        && m_heroVoicePlayer->playbackState() == QMediaPlayer::PlayingState;
#else
    heroVoicePlaying = m_heroVoicePlayer != nullptr
        && m_heroVoicePlayer->state() == QMediaPlayer::PlayingState;
#endif

    if (!heroVoicePlaying && m_skill3VoicePlayer != nullptr) {
        m_skill3VoicePlayer->stop();
        m_skill3VoicePlayer->setPosition(0);
        m_skill3VoicePlayer->play();
    }
}

void MainWindow::updateBulletWheelEffects()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;

    for (int i = m_bulletWheelBursts.size() - 1; i >= 0; --i) {
        BulletWheelBurst &burst = m_bulletWheelBursts[i];
        burst.elapsed += deltaMs;
        if (burst.elapsed < kBulletWheelBurstDurationMs) {
            continue;
        }

        m_bulletWheelBursts.removeAt(i);
    }
}

void MainWindow::castBulletWheel()
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF origin = heroCenter();
    constexpr qreal kBurstRange = 520.0;
    constexpr qreal kStepRadians = (kPi * 2.0) / 12.0;
    const qreal bulletSpeeds[12] = {14.0, 15.5, 17.0, 18.5, 20.0, 21.5, 23.0, 24.5, 26.0, 27.5, 29.0, 30.5};

    BulletWheelBurst burst;
    burst.center = origin;
    m_bulletWheelBursts.push_back(burst);

    for (int i = 0; i < 12; ++i) {
        const qreal radians = kStepRadians * i;
        const QPointF direction(std::cos(radians), std::sin(radians));
        const QPointF target = origin + direction * kBurstRange;
        const qreal speed = bulletSpeeds[i];

        switch (i) {
        case 0:
            m_bullets.push_back(new Bullet5(origin, target, speed));
            break;
        case 1:
            m_bullets.push_back(new Bullet6(origin, target, speed));
            break;
        case 2:
            m_bullets.push_back(new Bullet7(origin, target, speed));
            break;
        case 3:
            m_bullets.push_back(new Bullet8(origin, target, speed));
            break;
        case 4:
            m_bullets.push_back(new Bullet9(origin, target, speed));
            break;
        case 5:
            m_bullets.push_back(new Bullet10(origin, target, speed));
            break;
        case 6:
            m_bullets.push_back(new Bullet11(origin, target, speed));
            break;
        case 7:
            m_bullets.push_back(new Bullet12(origin, target, speed));
            break;
        case 8:
            m_bullets.push_back(new Bullet13(origin, target, speed));
            break;
        case 9:
            m_bullets.push_back(new Bullet14(origin, target, speed));
            break;
        case 10:
            m_bullets.push_back(new Bullet15(origin, target, speed));
            break;
        case 11:
            m_bullets.push_back(new Bullet16(origin, target, speed));
            break;
        default:
            break;
        }
    }
}

void MainWindow::castTreatment()
{
    if (myHero == nullptr || m_treatmentCooldownRemainingMs > 0.0) {
        return;
    }

    myHero->heal(kTreatmentHealAmount);
    m_treatmentCooldownRemainingMs = kTreatmentCooldownMs;
}

void MainWindow::castFlash()
{
    if (myHero == nullptr || m_flashCooldownRemainingMs > 0.0) {
        return;
    }

    const QPoint oldPos(myHero->Hero_x, myHero->Hero_y);
    const QPointF targetCenter = heroCenter() + normalized(m_skillDirection) * kFlashDistance;
    const int targetX = static_cast<int>(std::lround(std::clamp(targetCenter.x() - HERO_WIDTH / 2.0,
                                                                0.0,
                                                                std::max(0, worldWidth() - HERO_WIDTH) * 1.0)));
    const int targetY = static_cast<int>(std::lround(std::clamp(targetCenter.y() - HERO_HEIGHT / 2.0,
                                                                0.0,
                                                                std::max(0, worldHeight() - HERO_HEIGHT) * 1.0)));

    if (oldPos == QPoint(targetX, targetY)) {
        return;
    }

    myHero->setPosition(targetX, targetY);
    m_heroVelocity = QPointF(0.0, 0.0);
    m_heroMoving = false;
    m_heroMoveHoldElapsed = 0.0;
    m_flashCooldownRemainingMs = kFlashCooldownMs;
    m_flashEffectActive = true;
    m_flashEffectElapsed = 0.0;
    m_flashEffectStartPos = QPointF(oldPos);
    m_flashEffectEndPos = QPointF(targetX, targetY);
}

void MainWindow::updateHeroAnimation()
{
    if (myHero == nullptr) {
        return;
    }

    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    if (m_heroMoveHoldElapsed > 0.0) {
        m_heroMoveHoldElapsed = std::max(0.0, m_heroMoveHoldElapsed - deltaMs);
    }

    if (m_heroMoveHoldElapsed <= 0.0) {
        m_heroMoving = false;
        m_heroMoveAnimationElapsed = 0.0;
        m_heroMoveFrameIndex = 0;
        return;
    }

    if (m_heroMoveFrames.isEmpty()) {
        return;
    }

    m_heroMoving = true;
    m_heroMoveAnimationElapsed += deltaMs;
    if (m_heroMoveAnimationElapsed < kHeroMoveFrameDurationMs) {
        return;
    }

    m_heroMoveAnimationElapsed = 0.0;
    m_heroMoveFrameIndex = (m_heroMoveFrameIndex + 1) % m_heroMoveFrames.size();
}

void MainWindow::playSkillReadySound()
{
    if (m_skillReadyPlayer == nullptr) {
        return;
    }

    m_skillReadyPlayer->stop();
    m_skillReadyPlayer->setPosition(0);
    m_skillReadyPlayer->play();
}

void MainWindow::updateSkill3Effect()
{
    if (!m_skill3Active || myHero == nullptr) {
        return;
    }

    m_skill3Elapsed += m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;

    const qreal progress = std::clamp(m_skill3Elapsed / kSkill3DurationMs, 0.0, 1.0);
    const qreal sweepRadians = kSkill3SweepDegrees * kPi / 180.0;
    const qreal currentOffset = (progress - 0.5) * sweepRadians;
    const QPointF currentDirection = rotated(m_skill3BaseDirection, currentOffset);
    const QPointF start = heroCenter();
    const QPointF end = start + currentDirection * kSkill3Range;

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy *enemy = m_enemies.at(i);
        if (m_skill3HitEnemies.contains(enemy)) {
            continue;
        }

        const QRectF enemyRect = enemy->boundingRect();
        const qreal enemyRadius = std::max(enemyRect.width(), enemyRect.height()) / 2.0;
        const qreal distanceToLaser = distancePointToSegment(enemyRect.center(), start, end);
        if (distanceToLaser > kSkill3HitWidth / 2.0 + enemyRadius) {
            continue;
        }

        enemy->takeDamage(kSkill3Damage);
        m_skill3HitEnemies.push_back(enemy);
        if (!enemy->isDead()) {
            continue;
        }

        m_skill3HitEnemies.removeOne(enemy);
        delete enemy;
        m_enemies.removeAt(i);
    }

    if (progress >= 1.0) {
        m_skill3Active = false;
        m_skill3Elapsed = 0.0;
        m_skill3HitEnemies.clear();
    }
}

void MainWindow::clearSkillAim()
{
    m_activeSkill = SkillType::None;
    m_skillAiming = false;
    m_skillDragLength = 0.0;
    update();
}

void MainWindow::drawSkillArrow(QPainter &painter) const
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF center = heroCenter();
    const qreal arrowLength = std::min<qreal>(220.0, std::max<qreal>(110.0, m_skillDragLength));
    const QPointF arrowTip = center + m_skillDirection * arrowLength;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(255, 185, 46, 230));
    pen.setWidth(10);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawLine(center, arrowTip);

    const QPointF leftWing = arrowTip - m_skillDirection * 28.0 + rotated(m_skillDirection, kHalfPi) * 18.0;
    const QPointF rightWing = arrowTip - m_skillDirection * 28.0 + rotated(m_skillDirection, -kHalfPi) * 18.0;
    painter.setBrush(QBrush(QColor(255, 120, 40, 220)));
    QPolygonF arrowHead;
    arrowHead << arrowTip << leftWing << rightWing;
    painter.drawPolygon(arrowHead);

    painter.restore();
}

void MainWindow::drawHeroHealthBar(QPainter &painter) const
{
    if (myHero == nullptr) {
        return;
    }

    const int barX = myHero->Hero_x + (HERO_WIDTH - kHeroHpBarWidth) / 2;
    const int barY = myHero->Hero_y - 28;
    const QRect barRect(barX, barY, kHeroHpBarWidth, kHeroHpBarHeight);
    const QRect fillRect(barX + 14,
                         barY + 6,
                         static_cast<int>((kHeroHpBarWidth - 28) * std::clamp(myHero->hpRatio(), 0.0, 1.0)),
                         kHeroHpBarHeight - 12);

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(38, 54, 38, 210));
    painter.drawRoundedRect(barRect.adjusted(10, 4, -10, -4), 8, 8);
    painter.setBrush(QColor(74, 210, 88, 230));
    painter.drawRoundedRect(fillRect, 6, 6);

    if (!m_heroBloodPixmap.isNull()) {
        painter.drawPixmap(barRect, m_heroBloodPixmap);
    }
    painter.restore();
}

void MainWindow::drawSkill2Effects(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (const Skill2Explosion &explosion : m_skill2Explosions) {
        const qreal progress = std::clamp(explosion.elapsed / kSkill2ExplosionDurationMs, 0.0, 1.0);
        const qreal fade = 1.0 - progress;
        const qreal radius = 28.0 + kSkill2ExplosionMaxRadius * progress;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 190, 88, static_cast<int>(150 * fade)));
        painter.drawEllipse(explosion.center, radius, radius);

        painter.setBrush(QColor(255, 120, 48, static_cast<int>(170 * fade)));
        painter.drawEllipse(explosion.center, radius * 0.58, radius * 0.58);

        QPen ringPen(QColor(255, 238, 176, static_cast<int>(220 * fade)));
        ringPen.setWidth(5);
        painter.setPen(ringPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(explosion.center, radius * 0.82, radius * 0.82);

        QPen burstPen(QColor(255, 245, 214, static_cast<int>(200 * fade)));
        burstPen.setWidth(4);
        burstPen.setCapStyle(Qt::RoundCap);
        painter.setPen(burstPen);
        for (int i = 0; i < 8; ++i) {
            const qreal radians = (kPi * 2.0 / 8.0) * i + progress * 0.45;
            const QPointF direction(std::cos(radians), std::sin(radians));
            painter.drawLine(explosion.center + direction * (radius * 0.28),
                             explosion.center + direction * (radius + 16.0 * fade));
        }
    }

    painter.restore();
}

void MainWindow::drawBulletWheelEffects(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (const BulletWheelBurst &burst : m_bulletWheelBursts) {
        const qreal progress = std::clamp(burst.elapsed / kBulletWheelBurstDurationMs, 0.0, 1.0);
        const qreal fade = 1.0 - progress;
        const qreal radius = 26.0 + kBulletWheelBurstMaxRadius * progress;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 214, 112, static_cast<int>(110 * fade)));
        painter.drawEllipse(burst.center, radius, radius);

        painter.setBrush(QColor(255, 128, 56, static_cast<int>(145 * fade)));
        painter.drawEllipse(burst.center, radius * 0.42, radius * 0.42);

        QPen ringPen(QColor(255, 244, 186, static_cast<int>(220 * fade)));
        ringPen.setWidth(6);
        painter.setPen(ringPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(burst.center, radius * 0.8, radius * 0.8);

        QPen spokePen(QColor(255, 236, 170, static_cast<int>(205 * fade)));
        spokePen.setWidth(4);
        spokePen.setCapStyle(Qt::RoundCap);
        painter.setPen(spokePen);
        for (int i = 0; i < 12; ++i) {
            const qreal radians = (kPi * 2.0 / 12.0) * i + progress * 0.3;
            const QPointF direction(std::cos(radians), std::sin(radians));
            painter.drawLine(burst.center + direction * (radius * 0.28),
                             burst.center + direction * (radius + 20.0 * fade));
        }
    }

    painter.restore();
}

void MainWindow::drawFlashEffect(QPainter &painter) const
{
    if (!m_flashEffectActive || m_heroIdlePixmap.isNull()) {
        return;
    }

    const qreal progress = std::clamp(m_flashEffectElapsed / kFlashEffectDurationMs, 0.0, 1.0);
    const qreal fade = 1.0 - progress;
    const QPointF startCenter = m_flashEffectStartPos + QPointF(HERO_WIDTH / 2.0, HERO_HEIGHT / 2.0);
    const QPointF endCenter = m_flashEffectEndPos + QPointF(HERO_WIDTH / 2.0, HERO_HEIGHT / 2.0);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen trailPen(QColor(132, 255, 222, static_cast<int>(150 * fade)));
    trailPen.setWidth(18);
    trailPen.setCapStyle(Qt::RoundCap);
    painter.setPen(trailPen);
    painter.drawLine(startCenter, endCenter);

    QPen outerTrailPen(QColor(224, 255, 245, static_cast<int>(85 * fade)));
    outerTrailPen.setWidth(30);
    outerTrailPen.setCapStyle(Qt::RoundCap);
    painter.setPen(outerTrailPen);
    painter.drawLine(startCenter, endCenter);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(176, 255, 230, static_cast<int>(110 * fade)));
    painter.drawEllipse(endCenter, 28.0 + 16.0 * fade, 28.0 + 16.0 * fade);

    const qreal impactRadius = 24.0 + kFlashImpactMaxRadius * progress;
    painter.setBrush(QColor(205, 255, 242, static_cast<int>(105 * fade)));
    painter.drawEllipse(endCenter, impactRadius, impactRadius);
    painter.setBrush(QColor(255, 251, 214, static_cast<int>(155 * fade)));
    painter.drawEllipse(endCenter, impactRadius * 0.48, impactRadius * 0.48);

    QPen impactRingPen(QColor(255, 242, 182, static_cast<int>(220 * fade)));
    impactRingPen.setWidth(5);
    painter.setPen(impactRingPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(endCenter, impactRadius * 0.82, impactRadius * 0.82);

    QPen burstPen(QColor(245, 255, 228, static_cast<int>(185 * fade)));
    burstPen.setWidth(4);
    burstPen.setCapStyle(Qt::RoundCap);
    painter.setPen(burstPen);
    for (int i = 0; i < 6; ++i) {
        const qreal radians = (kPi * 2.0 / 6.0) * i + progress * 0.6;
        const QPointF direction(std::cos(radians), std::sin(radians));
        painter.drawLine(endCenter + direction * (impactRadius * 0.35),
                         endCenter + direction * (impactRadius + 18.0 * fade));
    }

    constexpr int ghostCount = 4;
    for (int i = 0; i < ghostCount; ++i) {
        const qreal ratio = ghostCount == 1 ? 1.0 : static_cast<qreal>(i) / static_cast<qreal>(ghostCount - 1);
        const QPointF ghostPos = m_flashEffectStartPos + (m_flashEffectEndPos - m_flashEffectStartPos) * ratio;
        const qreal ghostOpacity = (1.0 - ratio * 0.75) * 0.5 * fade;
        painter.setOpacity(ghostOpacity);
        if (m_heroFacingLeft) {
            painter.save();
            painter.translate(ghostPos.x() + HERO_WIDTH, ghostPos.y());
            painter.scale(-1.0, 1.0);
            painter.drawPixmap(QRectF(0.0, 0.0, HERO_WIDTH, HERO_HEIGHT),
                               m_heroIdlePixmap,
                               QRectF(0.0, 0.0, m_heroIdlePixmap.width(), m_heroIdlePixmap.height()));
            painter.restore();
        } else {
            painter.drawPixmap(QRectF(ghostPos.x(), ghostPos.y(), HERO_WIDTH, HERO_HEIGHT),
                               m_heroIdlePixmap,
                               QRectF(0.0, 0.0, m_heroIdlePixmap.width(), m_heroIdlePixmap.height()));
        }
    }

    painter.restore();
}

void MainWindow::drawSkill3Effect(QPainter &painter) const
{
    if (!m_skill3Active || myHero == nullptr || m_skill3LaserPixmap.isNull()) {
        return;
    }

    const qreal progress = std::clamp(m_skill3Elapsed / kSkill3DurationMs, 0.0, 1.0);
    const qreal sweepRadians = kSkill3SweepDegrees * kPi / 180.0;
    const qreal currentOffset = (progress - 0.5) * sweepRadians;
    const QPointF currentDirection = rotated(m_skill3BaseDirection, currentOffset);
    const qreal angleDegrees = std::atan2(currentDirection.y(), currentDirection.x()) * 180.0 / kPi;
    const QPointF start = heroCenter();

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.translate(start);
    painter.rotate(angleDegrees);
    painter.setOpacity(0.88);
    painter.drawPixmap(QRectF(0.0, -kSkill3VisualWidth / 2.0, kSkill3Range, kSkill3VisualWidth),
                       m_skill3LaserPixmap,
                       QRectF(0.0, 0.0, m_skill3LaserPixmap.width(), m_skill3LaserPixmap.height()));
    painter.restore();
}

QPointF MainWindow::heroCenter() const
{
    return QPointF(myHero->Hero_x + HERO_WIDTH / 2.0,
                   myHero->Hero_y + HERO_HEIGHT / 2.0);
}

QPointF MainWindow::cameraOffset() const
{
    if (myHero == nullptr) {
        return QPointF(0.0, 0.0);
    }

    const qreal maxCameraX = std::max(0, worldWidth() - width());
    const qreal maxCameraY = std::max(0, worldHeight() - height());
    const QPointF center = heroCenter();
    const qreal cameraX = std::clamp(center.x() - width() / 2.0, 0.0, maxCameraX * 1.0);
    const qreal cameraY = std::clamp(center.y() - height() / 2.0, 0.0, maxCameraY * 1.0);
    return QPointF(cameraX, cameraY);
}

int MainWindow::worldWidth() const
{
    return std::max(GAME_WIDTH, m_mapPixmap.isNull() ? 0 : m_mapPixmap.width());
}

int MainWindow::worldHeight() const
{
    return std::max(GAME_HEIGHT, m_mapPixmap.isNull() ? 0 : m_mapPixmap.height());
}
