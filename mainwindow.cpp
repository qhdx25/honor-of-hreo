#include "mainwindow.h"
#include "boss2enemy.h"
#include "boomerangbullet.h"
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
#include "dragonenemy.h"
#include "dragontornadobullet.h"
#include "enemybullet.h"
#include "enemybullet2.h"
#include "medicinepack.h"
#include "enemy.h"
#include "pet.h"
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
#include <QPushButton>
#include <QRandomGenerator>
#include <QRadialGradient>
#include <QTextOption>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>
#include <QBrush>
#include <QString>
#include <algorithm>
#include <cmath>
namespace {
using namespace GameConfig;
//路径拼接函数，自动补全
QString assetPath(const QString &fileName)
{
    return QStringLiteral("D:/develop/Qtproject/honor-of-hero/res/") + fileName;
}

// 根据地图尺寸计算三座防御塔的世界坐标。
QVector<QPointF> towerPositionsForWorld(int worldWidth, int worldHeight)
{
    return QVector<QPointF>{
        QPointF(worldWidth * 0.49, worldHeight * 0.16),
        QPointF(worldWidth * 0.66, worldHeight * 0.31),
        QPointF(worldWidth * 0.93, worldHeight * 0.46)
    };
}

//将任意方向向量标准化，方便统一乘速度和距离。
QPointF normalized(const QPointF &vector)
{
    const qreal length = std::hypot(vector.x(), vector.y());
    if (length <= 0.0001) {
        return QPointF(1.0, 0.0);
    }

    return QPointF(vector.x() / length, vector.y() / length);
}

//将一个向量旋转若干弧度，供扇形技能和扫射技能复用。
QPointF rotated(const QPointF &vector, qreal radians)
{
    const qreal cosValue = std::cos(radians);
    const qreal sinValue = std::sin(radians);
    return QPointF(vector.x() * cosValue - vector.y() * sinValue,
                   vector.x() * sinValue + vector.y() * cosValue);
}

//计算点到线段的最短距离，用于激光类技能的命中判定。
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

//Boss 敌人
bool isBossEnemyType(Enemy::Type type)
{
    return type == Enemy::Type::Dragon || type == Enemy::Type::Boss2 || type == Enemy::Type::Boss3;
}

//远程敌人生成敌方子弹。
bool isRangedEnemyType(Enemy::Type type)
{
    return type == Enemy::Type::Shooter || type == Enemy::Type::Boss3;
}

//击杀不同类型敌人会给英雄不同经验值。
int experienceForEnemyType(Enemy::Type type)
{
    return GameConfig::kEnemyExperienceByType.at(static_cast<std::size_t>(type));
}
}

// 主窗口构造函数：完成整个游戏场景和 UI 的初始化。
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initScene();//场景初始化函数
    myHero = new hero();//创建英雄类
    m_gameTimer = new QTimer(this);//定时器
    m_gameTimer->setInterval(16);//16毫秒
    QObject::connect(m_gameTimer, &QTimer::timeout, this, [this]() {
        updateBullets();//更新子弹
    }
                     );
    //主菜单背景音乐
    m_menuBgmPlayer = new QMediaPlayer(this);
    m_menuBgmPlayer->setMedia(QUrl::fromLocalFile(assetPath("startmusic.mp3")));//路径
    m_menuBgmPlayer->setVolume(45);//音量
    //循环播放
    QObject::connect(m_menuBgmPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia || m_menuBgmPlayer == nullptr) {
            return;
        }
        m_menuBgmPlayer->setPosition(0);
        m_menuBgmPlayer->play();
    });
    m_menuBgmPlayer->play();//播放
    //战斗背景音乐循环播放
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlayer->setMedia(QUrl::fromLocalFile(assetPath("music.mp3")));
    m_bgmPlayer->setVolume(40);
    QObject::connect(m_bgmPlayer, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        if (status != QMediaPlayer::EndOfMedia || m_bgmPlayer == nullptr) {
            return;
        }
        m_bgmPlayer->setPosition(0);
        m_bgmPlayer->play();
    });
    //失败音效播放器
    m_defeatAudioPlayer = new QMediaPlayer(this);
    m_defeatAudioPlayer->setMedia(QUrl::fromLocalFile(assetPath("defeat_audio.mp3")));
    m_defeatAudioPlayer->setVolume(100);
    //技能2音效
    m_skill2HitPlayer = new QMediaPlayer(this);
    m_skill2HitPlayer->setMedia(QUrl::fromLocalFile(assetPath("secondmusic.mp3")));
    m_skill2HitPlayer->setVolume(90);
    //英雄语音
    m_heroVoicePlayer = new QMediaPlayer(this);
    m_heroVoicePlayer->setMedia(QUrl::fromLocalFile(assetPath("heroword.mp3")));
    m_heroVoicePlayer->setVolume(100);
    //技能3语音
    m_skill3VoicePlayer = new QMediaPlayer(this);
    m_skill3VoicePlayer->setMedia(QUrl::fromLocalFile(assetPath("heroword2.mp3")));
    m_skill3VoicePlayer->setVolume(100);
    //英雄独白
    m_skillReadyPlayer = new QMediaPlayer(this);
    m_skillReadyPlayer->setMedia(QUrl::fromLocalFile(assetPath("ready_music.mp3")));
    m_skillReadyPlayer->setVolume(95);
    //火龙王登场音效
    m_dragonSpawnPlayer = new QMediaPlayer(this);
    m_dragonSpawnPlayer->setMedia(QUrl::fromLocalFile(assetPath("readtmusic.mp3")));
    m_dragonSpawnPlayer->setVolume(100);
    //火龙王死亡音效
    m_dragonDeathPlayer = new QMediaPlayer(this);
    m_dragonDeathPlayer->setMedia(QUrl::fromLocalFile(assetPath("secondmusic.mp3")));
    m_dragonDeathPlayer->setVolume(100);
    //失败动画
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
    //胜利动画
    m_victoryAudioPlayer = new QMediaPlayer(this);
    m_victoryAudioPlayer->setMedia(QUrl::fromLocalFile(assetPath("victory_audio.mp3")));
    m_victoryAudioPlayer->setVolume(100);
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
    //刷怪计时器
    m_enemyTimer = new QTimer(this);
    m_enemyTimer->setInterval(1800);
    QObject::connect(m_enemyTimer, &QTimer::timeout, this, [this]() { spawnEnemy(); });
    //加载游戏里所有图片资源：背景、按钮、角色、动画、地图、技能
    m_startMenuPixmap.load(assetPath("firstmenu.jpg"));    // 开始菜单背景
    m_startButtonPixmap.load(assetPath("startbutton.png"));// 开始按钮
    m_aboutButtonPixmap.load(assetPath("about.png"));      // 关于按钮
    m_iconButtonPixmap.load(assetPath("iconbutton.png"));  // 图标按钮
    m_iconShowPixmap.load(assetPath("iconshow.jpg"));      // 图标展示图
    m_heroIdlePixmap.load(assetPath("unmove.png"));        // 英雄待机图
    m_heroChangedPixmap.load(assetPath("herochanged.png"));// 英雄变身图
    m_heroBloodPixmap.load(assetPath("blood.png"));        // 英雄血条
    m_skill3LaserPixmap.load(assetPath("angela_skill3.png")); // 技能3激光
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
    };//加载英雄运动序列帧
    //加载游戏地图，并缩放
    const QPixmap rawMapPixmap(assetPath("hok_gorge_playfield_final_clean.png"));
    if (!rawMapPixmap.isNull()) {
        m_mapPixmap = rawMapPixmap.scaled(static_cast<int>(rawMapPixmap.width() * kMapDisplayScale),
                                          static_cast<int>(rawMapPixmap.height() * kMapDisplayScale),
                                          Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation);
    }
    if (!m_mapPixmap.isNull()) {
        myHero->setPosition(720, worldHeight() - 900);
    }
    m_pet = new Pet();//宠物
    m_crystal = new Crystal();//水晶
    m_medicinePack = new MedicinePack();//血包
    m_medicinePack->setCenter(QPointF(worldWidth() / 2.0 + 60.0, worldHeight() / 2.0 - 40.0));
    m_crystal->setCenter(QPointF(worldWidth() - 220.0, 190.0));
    for (int i = 0; i < 3; ++i) {
        m_towers.push_back(new Tower());
    }//三座防御塔
    const QVector<QPointF> towerPositions = towerPositionsForWorld(worldWidth(), worldHeight());
    for (int i = 0; i < m_towers.size() && i < towerPositions.size(); ++i) {
        m_towers.at(i)->setCenter(towerPositions.at(i));
    }
    //创建技能1图标
    const QPoint skill1Pos(1675, 920);
    m_skill1Icon = new SkillIconWidget(this);
    m_skill1Icon->setFocusPolicy(Qt::NoFocus);//不接收键盘焦点
    m_skill1Icon->setFrames(QVector<QString>{assetPath("2.png")});
    m_skill1Icon->setGeometry(skill1Pos.x(),//x
                              skill1Pos.y(),//y
                              SkillIconWidget::IconSize,//宽
                              SkillIconWidget::IconSize);//高
    //====================== 拖拽逻辑 ======================
    m_skill1Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill1);
    });//开始拖动技能图标
    m_skill1Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill1, dragOffset);
    });//拖动中
    m_skill1Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill1, dragOffset);
    });//释放技能
    m_skill1Icon->setCooldownState(0.0, kSkill1CooldownMs);//设置冷却时间
    //技能2
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
    //技能三
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
    //技能7
    const QPoint skill7Pos(width() - SkillIconWidget::IconSize - 36, 266);
    m_skill7Icon = new SkillIconWidget(this);
    m_skill7Icon->setFocusPolicy(Qt::NoFocus);
    m_skill7Icon->setFrames(QVector<QString>{assetPath("cuteevil.png")});
    m_skill7Icon->setGeometry(skill7Pos.x(),
                              skill7Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
    m_skill7Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill7);
    });
    m_skill7Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill7, dragOffset);
    });
    m_skill7Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill7, dragOffset);
    });
    m_skill7Icon->setCooldownState(0.0, 0.0);
    //技能6
    const QPoint skill6Pos(width() - SkillIconWidget::IconSize - 36,
                           266 + SkillIconWidget::IconSize + 18);
    m_skill6Icon = new SkillIconWidget(this);
    m_skill6Icon->setFocusPolicy(Qt::NoFocus);
    m_skill6Icon->setFrames(QVector<QString>{assetPath("skillicon6.png")});
    m_skill6Icon->setGeometry(skill6Pos.x(),
                              skill6Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
    m_skill6Icon->setDragStartedHandler([this]() {
        beginSkillAim(SkillType::Skill6);
    });
    m_skill6Icon->setDragMovedHandler([this](const QPoint &dragOffset) {
        updateSkillAim(SkillType::Skill6, dragOffset);
    });
    m_skill6Icon->setDragReleasedHandler([this](const QPoint &dragOffset) {
        releaseSkill(SkillType::Skill6, dragOffset);
    });
    m_skill6Icon->setCooldownState(0.0, kSkill6CooldownMs);
    //治疗术
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
//闪现
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
    //暂停键
    m_pauseButton = new QPushButton(QStringLiteral("暂停"), this);
    m_pauseButton->setGeometry(width() - 176, 26, 132, 50);
    m_pauseButton->setFocusPolicy(Qt::NoFocus);//不抢键盘焦点
    m_pauseButton->setCursor(Qt::PointingHandCursor);//鼠标放上去变小手
    m_pauseButton->setStyleSheet(QStringLiteral(
        "QPushButton {"
        " color: rgb(255, 247, 224);"//文字颜色
        " font-size: 22px;"//文字大小
        " font-weight: bold;"//加粗
        " border-radius: 18px;"//圆角
        " border: 2px solid rgba(255, 220, 140, 210);"//边框
        " background-color: rgba(72, 42, 18, 190);"//背景色
        " padding-bottom: 2px;"
        "}"
        //鼠标放上去的颜色
        "QPushButton:hover {"
        " background-color: rgba(106, 62, 24, 215);"
        "}"
        //鼠标按下的颜色
        "QPushButton:pressed {"
        " background-color: rgba(138, 82, 28, 225);"
        "}"
        "QPushButton:disabled {"
        " color: rgba(255, 244, 220, 150);"
        " border-color: rgba(255, 220, 140, 110);"
        " background-color: rgba(52, 36, 22, 130);"
        "}"));
    //暂停开始
    QObject::connect(m_pauseButton, &QPushButton::clicked, this, [this]() { togglePause(); });
    //一开始隐藏
    setGameplayUiVisible(false);
}
//析构函数
MainWindow::~MainWindow()
{//删除自己发射的子弹
    for (Bullet *bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();
    //删除敌人发射的子弹
    for (Bullet *bullet : m_enemyBullets) {
        delete bullet;
    }
    m_enemyBullets.clear();
    //删除宠物发射的子弹
    for (Bullet *bullet : m_petBullets) {
        delete bullet;
    }
    m_petBullets.clear();
    //删除所有敌人
    for (Enemy *enemy : m_enemies) {
        delete enemy;
    }
    m_enemies.clear();
    //删除所有防御塔
    for (Tower *tower : m_towers) {
        delete tower;
    }
    m_towers.clear();
    //删除宠物、血包、水晶、英雄
    delete m_pet;
    delete m_medicinePack;
    delete m_crystal;
    delete myHero;
}

//初始化窗口
void MainWindow::initScene()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);//大小
    setWindowTitle(GAME_TITLE);//标题
    setWindowIcon(QIcon(assetPath("icon.png")));//图标
    setFocusPolicy(Qt::StrongFocus);//焦点
    setFocus();//立刻让窗口获得焦点
}
//绘制函数
void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);//创建画笔
    //第一层：失败/胜利动画（全屏播放）
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
    //游戏开始前的主菜单
    if (!m_gameStarted) {
        if (!m_startMenuPixmap.isNull()) {
            painter.drawPixmap(rect(), m_startMenuPixmap);
        } else {
            painter.fillRect(rect(), QColor(18, 21, 27));
        }
        if (m_showAboutPage) {
            painter.save();
            painter.setRenderHint(QPainter::Antialiasing, true);//抗锯齿
            painter.fillRect(rect(), QColor(10, 16, 24, 156));//全屏半透明深色遮罩
            const QRectF panelRect = aboutPagePanelRect();//获取中间面板的位置大小
            //画中间的深色圆角面板
            painter.setPen(Qt::NoPen);//不要边框
            painter.setBrush(QColor(26, 34, 46, 220));//半透明深背景
            painter.drawRoundedRect(panelRect, 28.0, 28.0);//画圆角矩形
            //面板画一圈金色边框
            QPen panelPen(QColor(255, 220, 152, 210));//淡黄色边框
            panelPen.setWidth(3);//边框宽度3像素
            painter.setPen(panelPen);//设置画笔
            painter.setBrush(Qt::NoBrush);//不要填充
            painter.drawRoundedRect(panelRect.adjusted(1.5, 1.5, -1.5, -1.5), 28.0, 28.0);
            //设置标题文字样式
            QFont titleFont = painter.font();
            titleFont.setBold(true);//加粗
            titleFont.setPointSize(30);//字号30
            painter.setFont(titleFont);//应用字体
            painter.setPen(QColor(255, 245, 224));//米白色
            painter.drawText(QRectF(panelRect.left(), panelRect.top() + 28.0, panelRect.width(), 54.0),
                             Qt::AlignCenter,
                             QStringLiteral("关于游戏"));
            //设置正文字体
            QFont bodyFont = painter.font();
            bodyFont.setBold(false);
            bodyFont.setPointSize(17);
            painter.setFont(bodyFont);
            painter.setPen(QColor(240, 232, 214));
            //正文内容和显示区域
            const QString aboutText = aboutPageText();
            const QRectF textViewport = aboutPageTextViewportRect();
            //深色背景框
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(26, 34, 46, 245));
            painter.drawRoundedRect(textViewport.adjusted(-8.0, -8.0, 8.0, 8.0), 18.0, 18.0);
            painter.setPen(QColor(240, 232, 214));
            //计算文字高度
            const QFontMetricsF metrics(bodyFont);
            const QRectF contentBounds =
                metrics.boundingRect(QRectF(0.0, 0.0, textViewport.width(), 10000.0),
                                     Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                                     aboutText);
            //画文字
            painter.save();
            painter.setClipRect(textViewport);
            painter.drawText(QRectF(textViewport.left(),
                                    textViewport.top() - m_aboutScrollOffset,
                                    textViewport.width(),
                                    std::max(textViewport.height(), contentBounds.height() + 24.0)),
                             Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                             aboutText);
            painter.restore();
            //画右侧滚动条
            const qreal maxScroll = aboutPageMaxScroll();//最大能滚多高
            if (maxScroll > 0.0) {//如果内容超过显示区域，才画滚动条
                //画滚动条背景轨道
                const QRectF scrollTrack(textViewport.right() + 14.0,
                                         textViewport.top(),
                                         10.0,
                                         textViewport.height());
                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(255, 255, 255, 38));
                painter.drawRoundedRect(scrollTrack, 5.0, 5.0);
                //计算滚动条滑块高度
                const qreal thumbHeight = std::max<qreal>(54.0,
                                                          scrollTrack.height() * textViewport.height()
                                                              / std::max(textViewport.height(), contentBounds.height() + 24.0));
                const qreal travel = std::max<qreal>(0.0, scrollTrack.height() - thumbHeight);
                //计算滑块当前位置
                const qreal thumbTop = scrollTrack.top() + travel * (m_aboutScrollOffset / maxScroll);
                //画黄色滑块
                painter.setBrush(QColor(255, 215, 145, 210));
                painter.drawRoundedRect(QRectF(scrollTrack.left(), thumbTop, scrollTrack.width(), thumbHeight), 5.0, 5.0);
            }
            //返回按钮
            const QRect backRect = aboutBackButtonRect();
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(122, 78, 34, 225));
            painter.drawRoundedRect(backRect, 18, 18);
            painter.setPen(QColor(255, 243, 220));
            QFont backFont = painter.font();
            backFont.setBold(true);
            backFont.setPointSize(20);
            painter.setFont(backFont);
            painter.drawText(backRect, Qt::AlignCenter, QStringLiteral("返回"));
            painter.restore();
            return;
        }
        //图标界面
        if (m_showIconPage) {
            painter.save();
            painter.fillRect(rect(), Qt::white);//全屏白色
            if (!m_iconShowPixmap.isNull()) {//把图片缩放到窗口的 97% 大小
                const QSize targetSize = m_iconShowPixmap.size().scaled(static_cast<int>(width() * 0.97),
                                                                        static_cast<int>(height() * 0.97),
                                                                        Qt::KeepAspectRatio);
                //计算居中位置
                const QRect imageRect((width() - targetSize.width()) / 2,
                                      (height() - targetSize.height()) / 2,
                                      targetSize.width(),
                                      targetSize.height());
                painter.drawPixmap(imageRect, m_iconShowPixmap);
            }
            //返回按钮
            const QRect backRect = aboutBackButtonRect();
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(122, 78, 34, 225));
            painter.drawRoundedRect(backRect, 18, 18);
            painter.setPen(QColor(255, 243, 220));
            QFont backFont = painter.font();
            backFont.setBold(true);
            backFont.setPointSize(20);
            painter.setFont(backFont);
            painter.drawText(backRect, Qt::AlignCenter, QStringLiteral("返回"));
            painter.restore();//恢复画笔
            return;
        }
        //主菜单界面
        //开始游戏按钮
        const QRect buttonRect = startButtonRect();
        if (!m_startButtonPixmap.isNull()) {
            painter.drawPixmap(buttonRect, m_startButtonPixmap);
        } else {//如果没图片，就用画笔画一个圆角按钮
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(239, 179, 72));
            painter.drawRoundedRect(buttonRect, 18, 18);
            painter.setPen(QColor(58, 30, 8));
            painter.drawText(buttonRect, Qt::AlignCenter, QStringLiteral("关于游戏"));
        }
        //关于游戏按钮
        const QRect aboutRect = aboutButtonRect();
        if (!m_aboutButtonPixmap.isNull()) {
            painter.drawPixmap(aboutRect, m_aboutButtonPixmap);
        } else {
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(205, 160, 88));
            painter.drawRoundedRect(aboutRect, 18, 18);
            painter.setPen(QColor(58, 30, 8));
            painter.drawText(aboutRect, Qt::AlignCenter, QStringLiteral("关于游戏"));
        }
        //游戏图标按钮
        const QRect iconRect = iconButtonRect();
        if (!m_iconButtonPixmap.isNull()) {
            painter.drawPixmap(iconRect, m_iconButtonPixmap);
        } else {
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(188, 208, 230));
            painter.drawRoundedRect(iconRect, 18, 18);
            painter.setPen(QColor(52, 70, 96));
            painter.drawText(iconRect, Qt::AlignCenter, QStringLiteral("游戏图标"));
        }
        return;//菜单画完结束
    }
    //获取镜头当前位置
    const QPointF camera = cameraOffset();
//绘制地图
    if (!m_mapPixmap.isNull()) {
        painter.drawPixmap(QRectF(-camera.x(),//向左偏移
                                  -camera.y(),//向上偏移
                                  m_mapPixmap.width(),
                                  m_mapPixmap.height()),
                           m_mapPixmap,
                           QRectF(0.0, 0.0, m_mapPixmap.width(), m_mapPixmap.height()));
    }
    //如果地图图片加载失败，就用默认背景
    else {
        QPixmap bgPix(assetPath("background2.jpg"));
        painter.drawPixmap(0, 0, width(), height(), bgPix);
    }

    painter.save();
    //坐标转换：屏幕坐标到地图坐标
    painter.translate(-camera);//把整个画布偏移camera
// 画闪现特效
    if (m_flashEffectActive) {
        drawFlashEffect(painter);
    }
    //逻辑：先看有没有变身，再看有没有跑动，都不是用待机图
//画英雄
    if (myHero != nullptr) {
//判断英雄等级
        const bool transformed = myHero->level() >= 4 && !m_heroChangedPixmap.isNull();
//是否播放跑动动画
const bool useMoveFrame =
            !transformed//没有变身
            && m_heroMoving//英雄正在移动
            && !m_heroMoveFrames.isEmpty()//有跑步动画帧
            && !m_heroMoveFrames.at(m_heroMoveFrameIndex).isNull();//当前帧有效
        const QPixmap &heroPix = transformed//变身→变身图片
                                     ? m_heroChangedPixmap
                                     : (useMoveFrame ? m_heroMoveFrames.at(m_heroMoveFrameIndex) : m_heroIdlePixmap);
 //计算英雄画在屏幕的哪个位置
        const int drawWidth = useMoveFrame ? kHeroMoveFrameWidth : HERO_WIDTH;
        const int drawHeight = useMoveFrame ? kHeroMoveFrameHeight : HERO_HEIGHT;
        const int drawX = myHero->Hero_x - (drawWidth - HERO_WIDTH) / 2;
        const int drawY = myHero->Hero_y - (drawHeight - HERO_HEIGHT) / 2;
       //翻转图片
        if (m_heroFacingLeft && !heroPix.isNull()) {
            painter.save();
            painter.translate(drawX + drawWidth, drawY);//移动到右侧点
            painter.scale(-1.0, 1.0);//水平翻转
            painter.drawPixmap(0, 0, drawWidth, drawHeight, heroPix);
            painter.restore();
        }
        //向右
        else {
            painter.drawPixmap(drawX, drawY, drawWidth, drawHeight, heroPix);
        }
        //画血条
        drawHeroHealthBar(painter);
    }
//按固定顺序画物体、子弹、特效，保证层级正确
    //画血包
    if (m_medicinePack != nullptr) {
        m_medicinePack->paint(painter);
    }
//画宠物
    if (m_pet != nullptr) {
        m_pet->paint(painter);
    }
//画技能瞄准箭头
    if (m_skillAiming) {
        drawSkillArrow(painter);
    }
//子弹
    for (const Bullet *bullet : m_bullets) {
        bullet->paint(painter);
    }
//敌人发射的子弹
    for (const Bullet *bullet : m_enemyBullets) {
        bullet->paint(painter);
    }
//画宠物发射的子弹
    for (const Bullet *bullet : m_petBullets) {
        bullet->paint(painter);
    }
//画所有敌人
    for (const Enemy *enemy : m_enemies) {
        enemy->paint(painter);
    }
// 画所有防御塔
    for (const Tower *tower : m_towers) {
        tower->paint(painter);
    }
//画水晶
    if (m_crystal != nullptr) {
        m_crystal->paint(painter);
    }
//画技能2爆炸特效
    if (!m_skill2Explosions.isEmpty()) {
        drawSkill2Effects(painter);
    }
//耍赖技能特效
    if (!m_bulletWheelBursts.isEmpty()) {
        drawBulletWheelEffects(painter);
    }
//火龙王技能特效
    if (!m_dragonAttackWaves.isEmpty() || !m_dragonDeathBursts.isEmpty()) {
        drawDragonEffects(painter);
    }
//技能三特效
    if (m_skill3Active) {
        drawSkill3Effect(painter);
    }

    painter.restore();
//暂停界面
    if (m_gamePaused) {
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(), QColor(8, 12, 20, 122));
//中间面板的位置大小
        const QRectF panelRect(width() / 2.0 - 190.0, height() / 2.0 - 82.0, 380.0, 164.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(25, 32, 48, 210));
        painter.drawRoundedRect(panelRect, 24.0, 24.0);
//中间的深色圆角背景
        QPen panelPen(QColor(255, 220, 150, 190));
        panelPen.setWidth(3);
        painter.setPen(panelPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRoundedRect(panelRect.adjusted(1.5, 1.5, -1.5, -1.5), 24.0, 24.0);
//画一圈金色边框
        QFont titleFont = painter.font();
        titleFont.setBold(true);
        titleFont.setPointSize(28);
        painter.setFont(titleFont);
        painter.setPen(QColor(255, 245, 222));
        painter.drawText(panelRect.adjusted(0, 28, 0, -60), Qt::AlignHCenter, QStringLiteral("游戏暂停"));
//画标题文字
        QFont hintFont = painter.font();
        hintFont.setBold(false);
        hintFont.setPointSize(16);
        painter.setFont(hintFont);
        painter.setPen(QColor(240, 228, 204, 220));
        painter.drawText(panelRect.adjusted(24, 84, -24, -28),
                         Qt::AlignCenter,
                         QStringLiteral("点击右上角“继续”按钮返回战斗"));
        painter.restore();
    }
}
//键盘事件
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //ESC 键退出
    if (!m_gameStarted && (m_showAboutPage || m_showIconPage) && event->key() == Qt::Key_Escape) {
        m_showAboutPage = false;
        m_showIconPage = false;
        m_aboutScrollOffset = 0.0;
        update();
        event->accept();
        return;
    }
//如果游戏没开始、暂停、失败、胜利则不处理移动
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive || m_gamePaused) {
        QMainWindow::keyPressEvent(event);
        return;
    }
//处理WASD按键
    switch (event->key()) {
    case Qt::Key_W:
    case Qt::Key_A:
    case Qt::Key_S:
    case Qt::Key_D:
        m_pressedMovementKeys.insert(event->key());
        break;
    case Qt::Key_E://耍赖技能
        if (!event->isAutoRepeat()) {
            castBulletWheel();
        }
        break;
    default:
        break;
    }
    QMainWindow::keyPressEvent(event);
    update();//刷新画面
}

//键盘松开事件，取消对应方向键的持续移动状态
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive || m_gamePaused) {
        QMainWindow::keyPressEvent(event);
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

//鼠标点击事件
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    //菜单点击
    if (!m_gameStarted) {
        if (event->button() == Qt::LeftButton) {//处理左键
            if ((m_showAboutPage || m_showIconPage) && aboutBackButtonRect().contains(event->pos())) {
                m_showAboutPage = false;
                m_showIconPage = false;
                m_aboutScrollOffset = 0.0;
                update();
                return;
            }

            if (!m_showAboutPage && !m_showIconPage && startButtonRect().contains(event->pos())) {
                startGame();
                update();
                return;
            }

            if (!m_showAboutPage && !m_showIconPage && aboutButtonRect().contains(event->pos())) {
                m_showAboutPage = true;
                m_showIconPage = false;
                m_aboutScrollOffset = 0.0;
                update();
                return;
            }
           if (!m_showAboutPage && !m_showIconPage && iconButtonRect().contains(event->pos())) {
                m_showIconPage = true;
                m_showAboutPage = false;
                m_aboutScrollOffset = 0.0;
                update();
                return;
            }
        }

        QMainWindow::mousePressEvent(event);
        return;
    }

    if (m_defeatSequenceActive || m_victorySequenceActive || m_gamePaused) {
        QMainWindow::mousePressEvent(event);
        return;
    }
    //发射子弹
    if (event->button() == Qt::LeftButton && myHero != nullptr) {
        m_bullets.push_back(new Bullet(myHero->shootOrigin()//子弹从英雄枪口出发
                                       , QPointF(event->pos()) + cameraOffset()//鼠标指向的世界位置
                                       )
                            );
        update();
    }

    QMainWindow::mousePressEvent(event);
}

//鼠标滚轮事件：仅在关于页面中用于滚动文本内容。
void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (!m_gameStarted && m_showAboutPage) {
        const qreal maxScroll = aboutPageMaxScroll();
        if (maxScroll <= 0.0) {
            event->accept();
            return;
        }
        qreal delta = 0.0;
        if (!event->pixelDelta().isNull()) {
            delta = event->pixelDelta().y();
        } else {
            delta = event->angleDelta().y() / 2.0;
        }

        m_aboutScrollOffset = std::clamp(m_aboutScrollOffset - delta, 0.0, maxScroll);
        update();
        event->accept();
        return;
    }
    QMainWindow::wheelEvent(event);
}

//窗口失焦时清空移动输入，避免切出窗口后角色仍然持续移动。
void MainWindow::focusOutEvent(QFocusEvent *event)
{
    m_pressedMovementKeys.clear();
    m_heroVelocity = QPointF(0.0, 0.0);
    m_heroMoveHoldElapsed = 0.0;
    m_heroMoving = false;

    QMainWindow::focusOutEvent(event);
}
//开始函数
//开始一局新游戏：重置状态、启动定时器、切换音乐、显示战斗UI。
void MainWindow::startGame()
{
    if (m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    resetGameplayState();
    setPaused(false);
    m_showAboutPage = false;
    m_showIconPage = false;
    m_aboutScrollOffset = 0.0;
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
    //设置各种倒计时
    m_heroVoiceCountdownMs = kHeroVoiceInitialDelayMs;
    m_dragonSpawnCountdownMs = kDragonSpawnDelayMs;
    m_boss3SpawnCountdownMs = kBoss3SpawnDelayMs;
    //标记boss没有刷新
    m_dragonSpawned = false;
    m_boss2Spawned = false;
    setFocus();
}

//英雄死亡后进入失败结算：停止主循环，播放失败动画与音效。
void MainWindow::startDefeatSequence()
{//不能重复触发失败
    if (!m_gameStarted || m_defeatSequenceActive) {
        return;
    }

    setPaused(false);//取消暂停
    m_defeatSequenceActive = true;//标记：正在播放失败动画
    clearSkillAim();
    setGameplayUiVisible(false);
//停止所有游戏运行
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
//开始播放失败动画第一帧
    m_defeatFrameIndex = 0;
    m_currentDefeatFrame.load(m_defeatFramePaths.at(m_defeatFrameIndex));
    update();
//启动动画定时器
    if (m_defeatFrameTimer != nullptr && !m_defeatFrameTimer->isActive()) {
        m_defeatFrameTimer->start();
    }
}

//失败动画函数
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
//逐帧推进失败动画，播放完成后返回主菜单。
//胜利
void MainWindow::startVictorySequence()
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    setPaused(false);
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
//回到主菜单函数
//结束当前对局并回到主菜单，统一停止音频、定时器并恢复菜单状态。
void MainWindow::returnToMainMenu()
{//停止所有声音
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
    if (m_dragonSpawnPlayer != nullptr) {
        m_dragonSpawnPlayer->stop();
    }
    if (m_dragonDeathPlayer != nullptr) {
        m_dragonDeathPlayer->stop();
    }
//重置游戏所有数据
    resetGameplayState();
    //把所有状态恢复成菜单状态
    setPaused(false);
    m_gameStarted = false;
    m_showAboutPage = false;
    m_showIconPage = false;
    m_aboutScrollOffset = 0.0;
    m_defeatSequenceActive = false;
    m_victorySequenceActive = false;  
    setGameplayUiVisible(false);
    //播放菜单背景音乐
    if (m_menuBgmPlayer != nullptr) {
        m_menuBgmPlayer->setPosition(0);
        m_menuBgmPlayer->play();
    }
    update();
}
//将一局游戏中的动态对象和状态全部重置，供开局或回主菜单时复用。
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
    if (m_dragonSpawnPlayer != nullptr) {
        m_dragonSpawnPlayer->stop();
    }
    if (m_dragonDeathPlayer != nullptr) {
        m_dragonDeathPlayer->stop();
    }

    for (Bullet *bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();

    for (Bullet *bullet : m_enemyBullets) {
        delete bullet;
    }
    m_enemyBullets.clear();

    for (Bullet *bullet : m_petBullets) {
        delete bullet;
    }
    m_petBullets.clear();

    for (Enemy *enemy : m_enemies) {
        delete enemy;
    }
    m_enemies.clear();

    for (Tower *tower : m_towers) {
        tower->reset();
    }

    m_skill2Explosions.clear();
    m_bulletWheelBursts.clear();
    m_dragonAttackWaves.clear();
    m_dragonDeathBursts.clear();
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
    m_skill6CooldownRemainingMs = 0.0;
    m_flashCooldownRemainingMs = 0.0;
    m_treatmentCooldownRemainingMs = 0.0;
    m_flashEffectElapsed = 0.0;
    m_skillDragLength = 0.0;
    m_heroMoveAnimationElapsed = 0.0;
    m_heroMoveHoldElapsed = 0.0;
    m_heroVoiceCountdownMs = -1.0;
    m_dragonSpawnCountdownMs = kDragonSpawnDelayMs;
    m_boss3SpawnCountdownMs = kBoss3SpawnDelayMs;
    m_medicineRespawnCountdownMs = 0.0;
    m_skill3Elapsed = 0.0;
    m_heroMoveFrameIndex = 0;
    m_defeatFrameIndex = -1;
    m_victoryFrameIndex = -1;
    m_dragonSpawned = false;
    m_boss2Spawned = false;
    m_currentDefeatFrame = QPixmap();
    m_currentVictoryFrame = QPixmap();
    if (myHero != nullptr) {
        myHero->resetState();
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
    if (m_pet != nullptr) {
        m_pet->dismiss();
    }
    if (m_medicinePack != nullptr) {
        m_medicinePack->setCenter(QPointF(worldWidth() / 2.0 + 60.0, worldHeight() / 2.0 - 40.0));
        m_medicinePack->spawn();
    }
    const QVector<QPointF> towerPositions = towerPositionsForWorld(worldWidth(), worldHeight());
    for (int i = 0; i < m_towers.size() && i < towerPositions.size(); ++i) {
        m_towers.at(i)->reset();
        m_towers.at(i)->setCenter(towerPositions.at(i));
    }
//刷新所有界面
    updateSkillCooldowns();
    updateSkill2Effects();
    updateBulletWheelEffects();
    updateDragonEffects();
    updateMedicinePack();
    updateFlashState();
}

// 统一控制战斗技能图标和暂停按钮的显隐。
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
    if (m_skill6Icon != nullptr) {
        m_skill6Icon->setVisible(visible);
    }
    if (m_skill7Icon != nullptr) {
        m_skill7Icon->setVisible(visible);
    }
    if (m_flashIcon != nullptr) {
        m_flashIcon->setVisible(visible);
    }
    if (m_treatmentIcon != nullptr) {
        m_treatmentIcon->setVisible(visible);
    }
    if (m_pauseButton != nullptr) {
        m_pauseButton->setVisible(visible);
    }
}

//暂停按钮入口，负责在暂停和继续之间切换。
void MainWindow::togglePause()
{
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive) {
        return;
    }

    setPaused(!m_gamePaused);
}

//实际执行暂停或继续函数
void MainWindow::setPaused(bool paused)
{
    m_gamePaused = paused;//标记
    m_pressedMovementKeys.clear();//清空所有方向键
    m_heroVelocity = QPointF(0.0, 0.0);//速度归零
    m_heroMoveHoldElapsed = 0.0;
    m_heroMoving = false;//停止移动
    m_skillAiming = false;//取消瞄准
    m_activeSkill = SkillType::None;
    m_skillDragLength = 0.0;

    if (m_pauseButton != nullptr) {
        m_pauseButton->setText(paused ? QStringLiteral("继续") : QStringLiteral("暂停"));
    }
//禁用所有技能按钮
    const bool actionWidgetsEnabled = !paused;
    if (m_skill1Icon != nullptr) {
        m_skill1Icon->setEnabled(actionWidgetsEnabled);
    }
    if (m_skill2Icon != nullptr) {
        m_skill2Icon->setEnabled(actionWidgetsEnabled);
    }
    if (m_skill3Icon != nullptr) {
        m_skill3Icon->setEnabled(actionWidgetsEnabled);
    }
    if (m_skill6Icon != nullptr) {
        m_skill6Icon->setEnabled(actionWidgetsEnabled);
    }
    if (m_skill7Icon != nullptr) {
        m_skill7Icon->setEnabled(actionWidgetsEnabled);
    }
    if (m_flashIcon != nullptr) {
        m_flashIcon->setEnabled(actionWidgetsEnabled);
    }
    if (m_treatmentIcon != nullptr) {
        m_treatmentIcon->setEnabled(actionWidgetsEnabled);
    }
//如果是暂停
    if (paused) {
        if (m_gameTimer != nullptr) {
            m_gameTimer->stop();
        }
        if (m_enemyTimer != nullptr) {
            m_enemyTimer->stop();
        }
        update();
        return;
    }
//如果是继续
    if (m_gameStarted && !m_defeatSequenceActive && !m_victorySequenceActive) {
        if (m_gameTimer != nullptr && !m_gameTimer->isActive()) {
            m_gameTimer->start();
        }
        if (m_enemyTimer != nullptr && !m_enemyTimer->isActive()) {
            m_enemyTimer->start();
        }
        updateSkillCooldowns();
        updateSkill2Effects();
        updateFlashState();
    }

    update();
}
//开始游戏按钮
//这个地方我也不知道为什么有的时候开始游戏图标加载不出来，先加一个保底机制
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
//关于游戏按钮
QRect MainWindow::aboutButtonRect() const
{
    const QSize buttonSize = m_aboutButtonPixmap.isNull()
                                 ? QSize(320, 120)
                                 : m_aboutButtonPixmap.size().scaled(360, 160, Qt::KeepAspectRatio);
    const QRect startRect = startButtonRect();
    return QRect((width() - buttonSize.width()) / 2,
                 startRect.bottom() + 26,
                 buttonSize.width(),
                 buttonSize.height());
}
//图标按钮
QRect MainWindow::iconButtonRect() const
{
    const QSize buttonSize = m_iconButtonPixmap.isNull()
                                 ? QSize(320, 120)
                                 : m_iconButtonPixmap.size().scaled(360, 160, Qt::KeepAspectRatio);
    const QRect aboutRect = aboutButtonRect();
    return QRect((width() - buttonSize.width()) / 2,
                 aboutRect.bottom() + 26,
                 buttonSize.width(),
                 buttonSize.height());
}
//关于游戏图标按钮
QRect MainWindow::aboutBackButtonRect() const
{
    return QRect(width() / 2 - 110,
                 height() / 2 + 205,
                 220,
                 62);
}
//计算关于页面整个大面板的位置和大小
QRectF MainWindow::aboutPagePanelRect() const
{
    return QRectF(width() / 2.0 - 520.0, height() / 2.0 - 285.0, 1040.0, 570.0);
}
//计算文字显示区域函数
QRectF MainWindow::aboutPageTextViewportRect() const
{
    const QRectF panelRect = aboutPagePanelRect();
    return QRectF(panelRect.left() + 58.0,
                  panelRect.top() + 110.0,
                  panelRect.width() - 150.0,
                  panelRect.height() - 220.0);
}
//关于页面文字函数
QString MainWindow::aboutPageText() const
{
    return QString::fromUtf8(
        "写在前面:\n"
        "亲爱的玩家，当你看到这行字时，说明我的大作业已经安全落地了（୧(´▽`★)୭）\n"
        "我是NKU的一名大一学生，这是我的第一个游戏（嘿嘿）\n"
        "技术还不太完善，可能会有很多搞笑的bug(手速大神：这技能还没有我普攻快，是时候展示我的手速了)\n"
        "先叠一个甲，代码并非完全原创，有些函数学习借鉴了先辈的代码（比如大boss的火龙卷）(感恩(ˊ˘ˋ*)♡)(所以兄弟学校的同学们也不要焦虑)\n"
        "游戏内美术素材来源于《王者荣耀》，版权归腾讯公司所有，本人仅作学习使用，非商业用途、不会上架游戏、不用于任何盈利渠道。 如有侵权，请联系删(求放过▄█▀█●)\n"
        "怪物图片来源于《元气骑士》，版权归凉屋科技有限公司所有，本人仅作学习使用，非商业用途、不会上架游戏、不用于任何盈利渠道。 如有侵权，请联系删(求放过▄█▀█●×2)\n"
        "游戏图标来源：小红书作者，吃一口美味的蟹黄包(设计的图片好好看哦)。抠图软件：扣扣图\n"
        "游戏大佬和编程大佬们提出的问题我会虚心接受，认真学习,(=ＴェＴ=)挨骂)\n"
        "在这里感谢教我专业知识的老师和耐心帮助我修改代码的学长，谢谢你们՞˶･֊･˶՞\n"
        "特别鸣谢：xia书记，琼玉同学对我游戏的改进提供了很多宝贵建议。\n"
        "废话有点多，下面是一些提示ᜊ•͈⌔•͈ᜊ  go go go。\n"
        "1. 玩家操控安琪拉在峡谷地图中移动，使用普通攻击和技能清理敌人。\n\n"
        "2. 击杀小兵和 Boss 可以获得经验，安琪拉会升级，并逐步解锁更强的能力。\n\n"
        "3. 第六技能是可回收的回旋镖，第七技能可以召唤宠物协助作战，四级后还会变身。\n\n"
        "4. 游戏进行两分钟后会刷新 Boss，它们会释放火龙卷等远程攻击，需要灵活走位。\n\n"
        "5. 你的目标是在生存和成长中不断推进，摧毁敌方防御塔与水晶，获得胜利。\n\n"
        "6. E键是一个耍赖式大招，你可以在打不过时使用它（坏笑）\n\n"
        "                                     ——by秋来拾光\n"
        "                                      2026.4.1");
}

//计算最大滚动距离函数
qreal MainWindow::aboutPageMaxScroll() const
{
    QFont bodyFont = font();
    bodyFont.setPointSize(17);
    const QRectF textViewport = aboutPageTextViewportRect();
    const QFontMetricsF metrics(bodyFont);
    const QRectF contentBounds =
        metrics.boundingRect(QRectF(0.0, 0.0, textViewport.width(), 10000.0),
                             Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
                             aboutPageText());
    return std::max<qreal>(0.0, contentBounds.height() + 24.0 - textViewport.height());
}

//游戏的主循环函数
void MainWindow::updateBullets()
{//直接跳过不能运行的情况
    if (!m_gameStarted || m_defeatSequenceActive || m_victorySequenceActive || m_gamePaused) {
        return;
    }

    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
//更新所有基础状态
    updateHeroMovement();//英雄移动
    updateSkillCooldowns();//技能冷却
    updateSkill2Effects();//技能2特效
    updateBulletWheelEffects();//耍赖大招
    updateDragonEffects();//火龙王特效
    updateMedicinePack();//血包
    updateFlashState();//闪现
//独白更新
    if (m_heroVoiceCountdownMs > 0.0) {
        m_heroVoiceCountdownMs -= deltaMs;
        if (m_heroVoiceCountdownMs <= 0.0) {
            if (m_heroVoicePlayer != nullptr) {
                m_heroVoicePlayer->stop();
                m_heroVoicePlayer->setPosition(0);
                m_heroVoicePlayer->play();
            }
            m_heroVoiceCountdownMs += kHeroVoiceRepeatDelayMs;
        }
    }
//boss刷新
    if ((!m_dragonSpawned || !m_boss2Spawned) && m_dragonSpawnCountdownMs > 0.0) {
        m_dragonSpawnCountdownMs -= deltaMs;
        if (m_dragonSpawnCountdownMs <= 0.0) {
            if (!m_dragonSpawned) {
                spawnDragonEnemy();
                m_dragonSpawned = true;
            }
            if (!m_boss2Spawned) {
                spawnBoss2Enemy();
                m_boss2Spawned = true;
            }
            playDragonSpawnSound();
        }
    }

  //小boss刷新
    if (m_boss3SpawnCountdownMs > 0.0) {
        m_boss3SpawnCountdownMs -= deltaMs;
        if (m_boss3SpawnCountdownMs <= 0.0) {
            spawnBoss3Enemy();
            m_boss3SpawnCountdownMs += kBoss3SpawnDelayMs;
        }
    }

   //宠物更新
    if (m_pet != nullptr && m_pet->isActive() && myHero != nullptr) {
        m_pet->update(heroCenter(), deltaMs);

        Enemy *closestEnemy = nullptr;
        qreal closestDistance = 0.0;
        for (Enemy *enemy : m_enemies) {
            if (enemy == nullptr || enemy->isDead()) {
                continue;
            }

            const qreal distance = QLineF(m_pet->center(), enemy->boundingRect().center()).length();
            if (closestEnemy == nullptr || distance < closestDistance) {
                closestEnemy = enemy;
                closestDistance = distance;
            }
        }

        if (closestEnemy != nullptr && m_pet->tryShootAt(closestEnemy->boundingRect().center(), deltaMs)) {
            m_petBullets.push_back(new Bullet(m_pet->shootOrigin(),
                                              closestEnemy->boundingRect().center(),
                                              20.0,
                                              720.0,
                                              QSize(56, 56)));
        }
    }
//水晶高地塔攻击刷新
    if (m_crystal != nullptr && myHero != nullptr) {
        if (m_crystal->tryShootAt(heroCenter(), deltaMs)) {
            m_enemyBullets.push_back(new Bullet3(m_crystal->shootOrigin(), heroCenter(), GameConfig::kCrystalBulletSpeed, GameConfig::kCrystalBulletDistance));
        }
    }
    if (myHero != nullptr) {
        for (Tower *tower : m_towers) {
            if (tower != nullptr && tower->tryShootAt(heroCenter(), deltaMs)) {
                m_enemyBullets.push_back(new Bullet4(tower->shootOrigin(), heroCenter(), GameConfig::kTowerBulletSpeed, GameConfig::kTowerBulletDistance));
            }
        }
    }
//玩家子弹刷新
    for (int i = m_bullets.size() - 1; i >= 0; --i) {
        Bullet *bullet = m_bullets.at(i);
        BoomerangBullet *boomerangBullet = dynamic_cast<BoomerangBullet *>(bullet);
        bullet->update();

        if (bullet->isOutOfBounds(worldWidth(), worldHeight())
            || bullet->hasReachedMaxDistance()) {
            delete bullet;
            m_bullets.removeAt(i);
            continue;
        }

        const QRectF bulletRect = bullet->boundingRect();
        bool bulletConsumed = false;

      //防御塔被摧毁
        if (boomerangBullet == nullptr
            && m_crystal != nullptr
            && !m_crystal->isDead()
            && bulletRect.intersects(m_crystal->boundingRect())) {
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
            if (boomerangBullet != nullptr || tower == nullptr || tower->isDead() || !bulletRect.intersects(tower->boundingRect())) {
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

        //子弹打到敌人
        for (int e = m_enemies.size() - 1; e >= 0; --e) {
            Enemy *enemy = m_enemies.at(e);
            if (!bulletRect.intersects(enemy->boundingRect())) {
                continue;
            }
            if (boomerangBullet != nullptr && !boomerangBullet->canHitEnemy(enemy)) {
                continue;
            }
            // if (boomerangBullet != nullptr) {
            //     boomerangBullet->registerEnemyHit(enemy);
            // }
            const bool isSkill2Hit = dynamic_cast<Skill2Bullet *>(bullet) != nullptr;
            if (isSkill2Hit) {
                applySkill2AreaDamage(bulletRect.center(), bullet->velocity(), bullet->damage());
            } else {
                enemy->takeDamage(bullet->damage());
            }
            if (boomerangBullet != nullptr) {
                boomerangBullet->registerEnemyHit(enemy);
            }
            if (boomerangBullet == nullptr) {
                delete bullet;
                m_bullets.removeAt(i);
                bulletConsumed = true;
            }

            if (!isSkill2Hit && enemy->isDead()) {
                handleEnemyDefeat(e);
            }
            if (boomerangBullet == nullptr) {
                break;
            }
        }

        if (bulletConsumed) {
            continue;
        }
    }

    //敌人子弹打英雄
    if (myHero != nullptr) {
        const QRectF heroRect(myHero->Hero_x, myHero->Hero_y, HERO_WIDTH, HERO_HEIGHT);

        if (m_medicinePack != nullptr
            && m_medicinePack->isActive()
            && heroRect.intersects(m_medicinePack->boundingRect())) {
            myHero->heal(GameConfig::kMedicineHealAmount);
            m_medicinePack->consume();
            m_medicineRespawnCountdownMs = kMedicineRespawnDelayMs;
        }

        for (int i = m_enemyBullets.size() - 1; i >= 0; --i) {
            Bullet *bullet = m_enemyBullets.at(i);
            bullet->update();

            if (bullet->isOutOfBounds(worldWidth(), worldHeight())
                || bullet->hasReachedMaxDistance()) {
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

    //宠物子弹打敌人
    for (int i = m_petBullets.size() - 1; i >= 0; --i) {
        Bullet *bullet = m_petBullets.at(i);
        bullet->update();

        if (bullet->isOutOfBounds(worldWidth(), worldHeight())
            || bullet->hasReachedMaxDistance()) {
            delete bullet;
            m_petBullets.removeAt(i);
            continue;
        }

        const QRectF bulletRect = bullet->boundingRect();
        bool bulletConsumed = false;
        for (int e = m_enemies.size() - 1; e >= 0; --e) {
            Enemy *enemy = m_enemies.at(e);
            if (!bulletRect.intersects(enemy->boundingRect())) {
                continue;
            }

            enemy->takeDamage(bullet->damage());
            delete bullet;
            m_petBullets.removeAt(i);
            bulletConsumed = true;

            if (enemy->isDead()) {
                handleEnemyDefeat(e);
            }
            break;
        }

        if (bulletConsumed) {
            continue;
        }
    }

    //英雄死亡判断
    updateHeroAnimation();
    updateSkill3Effect();
    updateEnemies();
    if (myHero != nullptr && myHero->hp() <= 0) {
        startDefeatSequence();
        return;
    }
    update();
}

//更新各个主动技能的冷却时间，并同步到技能图标显示。
void MainWindow::updateSkillCooldowns()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    const qreal previousSkill1CooldownMs = m_skill1CooldownRemainingMs;
    const qreal previousSkill3CooldownMs = m_skill3CooldownRemainingMs;
    const qreal previousSkill6CooldownMs = m_skill6CooldownRemainingMs;
    const qreal previousTreatmentCooldownMs = m_treatmentCooldownRemainingMs;

    if (m_skill1CooldownRemainingMs > 0.0) {
        m_skill1CooldownRemainingMs = std::max(0.0, m_skill1CooldownRemainingMs - deltaMs);
    }
    if (m_skill3CooldownRemainingMs > 0.0) {
        m_skill3CooldownRemainingMs = std::max(0.0, m_skill3CooldownRemainingMs - deltaMs);
    }
    if (m_skill6CooldownRemainingMs > 0.0) {
        m_skill6CooldownRemainingMs = std::max(0.0, m_skill6CooldownRemainingMs - deltaMs);
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
    if (m_skill6Icon != nullptr) {
        m_skill6Icon->setCooldownState(m_skill6CooldownRemainingMs, kSkill6CooldownMs);
        m_skill6Icon->setEnabled(myHero != nullptr
                                 && myHero->level() >= 2
                                 && m_skill6CooldownRemainingMs <= 0.0);
    }
    if (m_skill7Icon != nullptr) {
        m_skill7Icon->setCooldownState(0.0, 0.0);
        m_skill7Icon->setEnabled(myHero != nullptr
                                 && myHero->level() >= 3
                                 && m_pet != nullptr
                                 && !m_pet->isActive());
    }
    if (m_treatmentIcon != nullptr) {
        m_treatmentIcon->setCooldownState(m_treatmentCooldownRemainingMs, kTreatmentCooldownMs);
        m_treatmentIcon->setEnabled(m_treatmentCooldownRemainingMs <= 0.0);
    }

    if ((previousSkill1CooldownMs > 0.0 && m_skill1CooldownRemainingMs <= 0.0)
        || (previousSkill3CooldownMs > 0.0 && m_skill3CooldownRemainingMs <= 0.0)
        || (previousSkill6CooldownMs > 0.0 && m_skill6CooldownRemainingMs <= 0.0)
        || (previousTreatmentCooldownMs > 0.0 && m_treatmentCooldownRemainingMs <= 0.0)) {
        playSkillReadySound();
    }
}

//更新二技能爆炸特效的生命周期，同时维护技能2冷却。
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

//更新闪现的冷却和闪现残影特效持续时间。
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

//更新Boss攻击波纹和死亡爆炸这两类特效的存在时间。
void MainWindow::updateDragonEffects()
{
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;

    for (int i = m_dragonAttackWaves.size() - 1; i >= 0; --i) {
        DragonAttackWave &wave = m_dragonAttackWaves[i];
        wave.elapsed += deltaMs;
        if (wave.elapsed < kDragonAttackWaveDurationMs) {
            continue;
        }

        m_dragonAttackWaves.removeAt(i);
    }

    for (int i = m_dragonDeathBursts.size() - 1; i >= 0; --i) {
        DragonDeathBurst &burst = m_dragonDeathBursts[i];
        burst.elapsed += deltaMs;
        if (burst.elapsed < kDragonDeathBurstDurationMs) {
            continue;
        }

        m_dragonDeathBursts.removeAt(i);
    }
}

//处理药包的重生计时
void MainWindow::updateMedicinePack()
{
    if (m_medicinePack == nullptr || m_medicinePack->isActive()) {
        return;
    }

    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;
    if (m_medicineRespawnCountdownMs > 0.0) {
        m_medicineRespawnCountdownMs = std::max(0.0, m_medicineRespawnCountdownMs - deltaMs);
    }

    if (m_medicineRespawnCountdownMs <= 0.0) {
        m_medicinePack->spawn();
    }
}
//移动函数
//根据当前按下的 WASD 键计算速度，并更新英雄在地图中的位置。
void MainWindow::updateHeroMovement()
{
    if (myHero == nullptr) {
        return;
    }
//初始化输入方向
    QPointF inputDirection(0.0, 0.0);
    //监测按键
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
//计算输入长度
    const qreal inputLength = std::hypot(inputDirection.x(), inputDirection.y());
    if (inputLength > 0.0001) {
        //设置英雄朝向
        if (inputDirection.x() < -0.1) {
            m_heroFacingLeft = true;
        } else if (inputDirection.x() > 0.1) {
            m_heroFacingLeft = false;
        }
//计算目标速度
        const QPointF targetVelocity =
            QPointF(inputDirection.x() / inputLength, inputDirection.y() / inputLength) * myHero->Hero_speed;
        //平滑加速
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
//按照预设类型池和出生点，生成一个普通敌人。
void MainWindow::spawnEnemy()
{
    if (myHero == nullptr) {
        return;
    }

    const QVector<Enemy::Type> spawnPool{
        Enemy::Type::Scout,
        Enemy::Type::Warrior,
        Enemy::Type::Mage,
        Enemy::Type::Tank,
        Enemy::Type::Assassin,
        Enemy::Type::Shooter,
        Enemy::Type::Shooter,
        Enemy::Type::Shooter
    };
    const Enemy::Type type =
        spawnPool.at(static_cast<int>(QRandomGenerator::global()->bounded(static_cast<quint32>(spawnPool.size()))));
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

//生成龙Boss。
void MainWindow::spawnDragonEnemy()
{
    if (myHero == nullptr) {
        return;
    }

    const QPoint dragonSpawnPoint(worldWidth() - 820, worldHeight() / 2 - 180);
    m_enemies.push_back(new DragonEnemy(QPointF(dragonSpawnPoint)));
}

//生成第二个Boss。
void MainWindow::spawnBoss2Enemy()
{
    if (myHero == nullptr) {
        return;
    }

    const QPoint boss2SpawnPoint(worldWidth() - 930, worldHeight() / 2 + 70);
    m_enemies.push_back(new Boss2Enemy(QPointF(boss2SpawnPoint)));
}

//生成第三类远程Boss。
void MainWindow::spawnBoss3Enemy()
{
    if (myHero == nullptr) {
        return;
    }

    const QPoint boss3SpawnPoint(worldWidth() - 1040, worldHeight() / 2 - 40);
    m_enemies.push_back(new Enemy(Enemy::Type::Boss3, QPointF(boss3SpawnPoint)));
}

//更新所有敌人的AI：靠近、攻击、发射子弹，以及越界清理。
void MainWindow::updateEnemies()
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF center = heroCenter();
    const qreal deltaMs = m_gameTimer != nullptr ? m_gameTimer->interval() : 16.0;

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy *enemy = m_enemies.at(i);
        const bool isBoss = isBossEnemyType(enemy->type());
        const bool isRangedEnemy = isRangedEnemyType(enemy->type());
        const bool isBoss3 = enemy->type() == Enemy::Type::Boss3;
        const qreal followDistance = enemy->type() == Enemy::Type::Boss3 ? GameConfig::kBoss3FollowDistance : (isBoss ? GameConfig::kBossFollowDistance : 0.0);
        const qreal distanceToHero = QLineF(enemy->boundingRect().center(), center).length();

        if (!isBoss && !isRangedEnemy && !enemy->reachesTarget(center)) {
            enemy->updateToward(center);
        } else if (isRangedEnemy && !enemy->reachesTarget(center)) {
            enemy->updateToward(center);
        } else if (isBoss && distanceToHero > followDistance) {
            enemy->updateToward(center);
        } else if (enemy->tryAttackTarget(center, deltaMs)) {
            if (isBoss3) {
                const QPointF attackOrigin = enemy->boundingRect().center();
                m_enemyBullets.push_back(new EnemyBullet2(attackOrigin,
                                                          center,
                                                          enemy->attackDamage(),
                                                          16.0,
                                                          3200.0));
            } else if (isBoss) {
                const QPointF attackOrigin = enemy->boundingRect().center();
                m_enemyBullets.push_back(new DragonTornadoBullet(attackOrigin,
                                                                center,
                                                                enemy->attackDamage(),
                                                                GameConfig::kDragonTornadoSpeed,
                                                                GameConfig::kDragonTornadoDistance));

                DragonAttackWave wave;
                wave.center = attackOrigin;
                wave.rotationSeed = QRandomGenerator::global()->generateDouble() * 360.0;
                wave.scale = 0.92 + QRandomGenerator::global()->generateDouble() * 0.28;
                m_dragonAttackWaves.push_back(wave);
            } else if (isRangedEnemy) {
                const QPointF attackOrigin = enemy->boundingRect().center();
                m_enemyBullets.push_back(new EnemyBullet(attackOrigin,
                                                         center,
                                                         enemy->attackDamage(),
                                                         14.0,
                                                         2600.0));
            } else {
                myHero->takeDamage(enemy->attackDamage());
            }
        }
        enemy->updateEnteredState(worldWidth(), worldHeight());

        const bool shouldRemove =
            enemy->hasEnteredScreen() && enemy->isOutOfBounds(worldWidth(), worldHeight());

        if (shouldRemove) {
            delete enemy;
            m_enemies.removeAt(i);
        }
    }

    resolveEnemyOverlap();
}

// 简单处理敌人之间的重叠，避免多个敌人完全堆在一起。
void MainWindow::resolveEnemyOverlap()
{
    for (int i = 0; i < m_enemies.size(); ++i) {
        Enemy *enemyA = m_enemies.at(i);
        if (enemyA == nullptr || enemyA->isDead()) {
            continue;
        }

        for (int j = i + 1; j < m_enemies.size(); ++j) {
            Enemy *enemyB = m_enemies.at(j);
            if (enemyB == nullptr || enemyB->isDead()) {
                continue;
            }

            QRectF rectA = enemyA->boundingRect();
            QRectF rectB = enemyB->boundingRect();
            if (!rectA.intersects(rectB)) {
                continue;
            }

            QPointF centerA = rectA.center();
            QPointF centerB = rectB.center();
            QPointF separation = centerB - centerA;
            qreal separationLength = std::hypot(separation.x(), separation.y());
            if (separationLength <= 0.0001) {
                separation = QPointF(1.0, 0.0);
                separationLength = 1.0;
            }

            const QPointF direction(separation.x() / separationLength,
                                    separation.y() / separationLength);
            const qreal radiusA = std::max(rectA.width(), rectA.height()) * 0.5;
            const qreal radiusB = std::max(rectB.width(), rectB.height()) * 0.5;
            const qreal desiredSpacing = (radiusA + radiusB) * 0.92;
            const qreal penetration = desiredSpacing - separationLength;
            if (penetration <= 0.0) {
                continue;
            }

            const qreal softPushDistance = std::clamp(penetration * 0.35, 2.0, 12.0);
            const QPointF pushOffset = direction * softPushDistance;

            rectA.translate(-pushOffset.x() * 0.5, -pushOffset.y() * 0.5);
            rectB.translate(pushOffset.x() * 0.5, pushOffset.y() * 0.5);

            const qreal clampedAX = std::clamp(rectA.left(), 0.0, std::max(0, worldWidth()) - rectA.width() * 1.0);
            const qreal clampedAY = std::clamp(rectA.top(), 0.0, std::max(0, worldHeight()) - rectA.height() * 1.0);
            const qreal clampedBX = std::clamp(rectB.left(), 0.0, std::max(0, worldWidth()) - rectB.width() * 1.0);
            const qreal clampedBY = std::clamp(rectB.top(), 0.0, std::max(0, worldHeight()) - rectB.height() * 1.0);

            enemyA->setCenter(QPointF(clampedAX + rectA.width() / 2.0,
                                      clampedAY + rectA.height() / 2.0));
            enemyB->setCenter(QPointF(clampedBX + rectB.width() / 2.0,
                                      clampedBY + rectB.height() / 2.0));
        }
    }
}

//敌人死亡后的统一处理：加经验、播放特效、从数组删除。
void MainWindow::handleEnemyDefeat(int enemyIndex)
{
    if (enemyIndex < 0 || enemyIndex >= m_enemies.size()) {
        return;
    }

    Enemy *enemy = m_enemies.at(enemyIndex);
    if (enemy == nullptr) {
        m_enemies.removeAt(enemyIndex);
        return;
    }

    m_skill3HitEnemies.removeOne(enemy);
    if (myHero != nullptr) {
        myHero->gainExperience(experienceForEnemyType(enemy->type()));
    }
    if (isBossEnemyType(enemy->type())) {
        DragonDeathBurst burst;
        burst.center = enemy->boundingRect().center();
        m_dragonDeathBursts.push_back(burst);
        playDragonDeathSound();
    }

    delete enemy;
    m_enemies.removeAt(enemyIndex);
}

//播放Boss出场音效。
void MainWindow::playDragonSpawnSound()
{
    if (m_dragonSpawnPlayer == nullptr) {
        return;
    }

    m_dragonSpawnPlayer->stop();
    m_dragonSpawnPlayer->setPosition(0);
    m_dragonSpawnPlayer->play();
}

//播放Boss死亡音效。
void MainWindow::playDragonDeathSound()
{
    if (m_dragonDeathPlayer == nullptr) {
        return;
    }

    m_dragonDeathPlayer->stop();
    m_dragonDeathPlayer->setPosition(0);
    m_dragonDeathPlayer->play();
}

//技能拖拽开始时进入瞄准状态。
void MainWindow::beginSkillAim(SkillType skill)
{
    m_activeSkill = skill;
    m_skillAiming = false;
    m_skillDragLength = 0.0;
    update();
}

//根据鼠标拖拽方向实时更新技能瞄准箭头。
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

//松开技能图标后根据拖拽方向真正释放技能。
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
    case SkillType::Skill6:
        castSkill6();
        break;
    case SkillType::Skill7:
        castSkill7();
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

//一技能
void MainWindow::castSkill1()
{
    if (myHero == nullptr || m_skill1CooldownRemainingMs > 0.0) {
        return;
    }

    const QPointF origin = myHero->shootOrigin();
    const int bulletCount = GameConfig::kSkill1BulletCount;
    const qreal spreadDegrees = GameConfig::kSkill1SpreadDegrees;
    const qreal stepDegrees = spreadDegrees / (bulletCount - 1);

    for (int i = 0; i < bulletCount; ++i) {
        const qreal degrees = -spreadDegrees / 2.0 + i * stepDegrees;
        const qreal radians = degrees * kPi / 180.0;
        const QPointF direction = rotated(m_skillDirection, radians);
        const QPointF target = origin + direction * GameConfig::kSkill1TargetDistance;
        m_bullets.push_back(new Bullet(origin, target, GameConfig::kSkill1BulletSpeed, GameConfig::kSkill1BulletDistance, QSize(GameConfig::kSkill1BulletWidth, GameConfig::kSkill1BulletHeight)));
    }

    m_skill1CooldownRemainingMs = kSkill1CooldownMs;
}

// 二技能
void MainWindow::castSkill2()
{
    if (myHero == nullptr || m_skill2CooldownRemainingMs > 0.0) {
        return;
    }

    const QPointF origin = myHero->shootOrigin();
    const QPointF target = origin + m_skillDirection * GameConfig::kSkill2TargetDistance;
    m_bullets.push_back(new Skill2Bullet(origin, target, GameConfig::kSkill2BulletSpeed, GameConfig::kSkill2BulletDistance));
    m_skill2CooldownRemainingMs = kSkill2CooldownMs;
}

// 二技能命中后对范围内敌人结算伤害和击退。
void MainWindow::applySkill2AreaDamage(const QPointF &impactCenter, const QPointF &impactDirection, int damage)
{
    QVector<int> defeatedEnemyIndices;
    defeatedEnemyIndices.reserve(m_enemies.size());

    for (int i = 0; i < m_enemies.size(); ++i) {
        Enemy *enemy = m_enemies.at(i);
        if (enemy == nullptr || enemy->isDead()) {
            continue;
        }

        const QRectF enemyRect = enemy->boundingRect();
        const qreal enemyRadius = std::max(enemyRect.width(), enemyRect.height()) / 2.0;
        const qreal distanceToImpact = QLineF(impactCenter, enemyRect.center()).length();
        if (distanceToImpact > kSkill2DamageRadius + enemyRadius) {
            continue;
        }

        enemy->takeDamage(damage);
        enemy->applyKnockback(impactDirection, GameConfig::kSkill2KnockbackDistance, worldWidth(), worldHeight());
        if (enemy->isDead()) {
            defeatedEnemyIndices.push_back(i);
        }
    }

    Skill2Explosion explosion;
    explosion.center = impactCenter;
    m_skill2Explosions.push_back(explosion);

    if (m_skill2HitPlayer != nullptr) {
        m_skill2HitPlayer->stop();
        m_skill2HitPlayer->setPosition(0);
        m_skill2HitPlayer->play();
    }

    for (int i = defeatedEnemyIndices.size() - 1; i >= 0; --i) {
        handleEnemyDefeat(defeatedEnemyIndices.at(i));
    }
}

// 三技能
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
    heroVoicePlaying = m_heroVoicePlayer != nullptr
        && m_heroVoicePlayer->state() == QMediaPlayer::PlayingState;

    if (!heroVoicePlaying && m_skill3VoicePlayer != nullptr) {
        m_skill3VoicePlayer->stop();
        m_skill3VoicePlayer->setPosition(0);
        m_skill3VoicePlayer->play();
    }
}

// 六技能
void MainWindow::castSkill6()
{
    if (myHero == nullptr || myHero->level() < 2 || m_skill6CooldownRemainingMs > 0.0) {
        return;
    }

    const QPointF origin = myHero->shootOrigin();
    const QPointF target = origin + m_skillDirection * GameConfig::kSkill6TargetDistance;
    m_bullets.push_back(new BoomerangBullet(origin,
                                            target,
                                            [this]() {
                                                return myHero != nullptr ? myHero->shootOrigin() : QPointF();
                                            }));
    m_skill6CooldownRemainingMs = kSkill6CooldownMs;
}

//七技能
void MainWindow::castSkill7()
{
    if (myHero == nullptr || myHero->level() < 3 || m_pet == nullptr || m_pet->isActive()) {
        return;
    }

    m_pet->summon(heroCenter());
    if (m_skill7Icon != nullptr) {
        m_skill7Icon->setEnabled(false);
    }
}

//更新E键弹幕特效的持续时间。
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

//E 键技能
void MainWindow::castBulletWheel()
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF origin = heroCenter();
    const qreal kBurstRange = GameConfig::kBulletWheelBurstRange;
    const qreal kStepRadians = (kPi * 2.0) / GameConfig::kBulletWheelProjectileCount;
    const auto &bulletSpeeds = GameConfig::kBulletWheelSpeeds;

    BulletWheelBurst burst;
    burst.center = origin;
    m_bulletWheelBursts.push_back(burst);

    for (int i = 0; i < GameConfig::kBulletWheelProjectileCount; ++i) {
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

//治疗技能
void MainWindow::castTreatment()
{
    if (myHero == nullptr || m_treatmentCooldownRemainingMs > 0.0) {
        return;
    }

    myHero->heal(GameConfig::kTreatmentHealAmount);
    m_treatmentCooldownRemainingMs = kTreatmentCooldownMs;
}

//闪现技能：沿当前拖拽方向瞬间位移一段距离。
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

//根据角色是否移动切换跑步帧或待机帧。
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

//某个技能冷却转好时播放提示音。
void MainWindow::playSkillReadySound()
{
    if (m_skillReadyPlayer == nullptr) {
        return;
    }

    m_skillReadyPlayer->stop();
    m_skillReadyPlayer->setPosition(0);
    m_skillReadyPlayer->play();
}

//在技能3持续期间，按扫射方向逐帧计算命中并结算伤害。
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

        handleEnemyDefeat(i);
    }

    if (progress >= 1.0) {
        m_skill3Active = false;
        m_skill3Elapsed = 0.0;
        m_skill3HitEnemies.clear();
    }
}

//结束瞄准状态，清除箭头和拖拽长度。
void MainWindow::clearSkillAim()
{
    m_activeSkill = SkillType::None;
    m_skillAiming = false;
    m_skillDragLength = 0.0;
    update();
}

//绘制技能拖拽时的方向箭头。
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

//绘制英雄的血条和经验条。
void MainWindow::drawHeroHealthBar(QPainter &painter) const
{
    if (myHero == nullptr) {
        return;
    }

    const int worldBarX = myHero->Hero_x + (HERO_WIDTH - 170) / 2;
    const int worldBarY = myHero->Hero_y - 28;
    const int expBarX = myHero->Hero_x + (HERO_WIDTH - 156) / 2;
    const int expBarY = worldBarY - 14;
    const QRect worldBarRect(worldBarX, worldBarY, 170, 26);
    const QRect expBarRect(expBarX, expBarY, 156, 12);
    const QRect worldFillRect(worldBarX + 14,
                              worldBarY + 6,
                              static_cast<int>((170 - 28) * std::clamp(myHero->hpRatio(), 0.0, 1.0)),
                              26 - 12);
    const QRect expFillRect(expBarX + 3,
                            expBarY + 3,
                            static_cast<int>((156 - 6) * std::clamp(myHero->experienceRatio(), 0.0, 1.0)),
                            12 - 6);
    const QPointF camera = cameraOffset();
    const QRect panelRect(static_cast<int>(18 + camera.x()),
                          static_cast<int>(18 + camera.y()),
                          430,
                          92);
    const QRect labelRect(panelRect.left() + 18, panelRect.top() + 10, panelRect.width() - 36, 24);
    const QRect barRect(panelRect.left() + 18, panelRect.top() + 38, kHeroHpBarWidth, kHeroHpBarHeight);
    const QRect fillRect(barRect.left() + 5,
                         barRect.top() + 5,
                         static_cast<int>((barRect.width() - 10) * std::clamp(myHero->hpRatio(), 0.0, 1.0)),
                         barRect.height() - 10);
    const QRect valueRect(barRect.right() + 14, barRect.top() - 1, 52, barRect.height());

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);

    painter.setBrush(QColor(16, 34, 78, 220));
    painter.drawRoundedRect(expBarRect, 6, 6);
    painter.setBrush(QColor(76, 170, 255, 235));
    painter.drawRoundedRect(expFillRect, 4, 4);

    QPen expBorderPen(QColor(176, 228, 255, 230));
    expBorderPen.setWidth(2);
    painter.setPen(expBorderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(expBarRect.adjusted(0, 0, -1, -1), 6, 6);

    painter.setPen(QColor(235, 246, 255));
    painter.drawText(expBarRect.adjusted(0, -16, 0, -2),
                     Qt::AlignCenter,
                     QStringLiteral("Lv.%1").arg(myHero->level()));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(38, 54, 38, 210));
    painter.drawRoundedRect(worldBarRect.adjusted(10, 4, -10, -4), 8, 8);
    painter.setBrush(QColor(74, 210, 88, 230));
    painter.drawRoundedRect(worldFillRect, 6, 6);

    if (!m_heroBloodPixmap.isNull()) {
        painter.drawPixmap(worldBarRect, m_heroBloodPixmap);
    }

    painter.setBrush(QColor(12, 14, 18, 205));
    painter.drawRoundedRect(panelRect, 18, 18);

    painter.setBrush(QColor(75, 18, 24, 225));
    painter.drawRoundedRect(barRect, 14, 14);

    painter.setBrush(QColor(224, 36, 48, 235));
    painter.drawRoundedRect(fillRect, 10, 10);

    painter.setPen(QColor(255, 255, 255, 235));
    QFont labelFont = painter.font();
    labelFont.setBold(true);
    labelFont.setPointSize(11);
    painter.setFont(labelFont);
    painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("英雄生命值"));

    QFont hpFont = painter.font();
    hpFont.setBold(true);
    hpFont.setPointSize(14);
    painter.setFont(hpFont);
    painter.drawText(barRect, Qt::AlignCenter, QStringLiteral("%1 / %2").arg(myHero->hp()).arg(myHero->maxHp()));

    QFont valueFont = painter.font();
    valueFont.setBold(true);
    valueFont.setPointSize(16);
    painter.setFont(valueFont);
    painter.setPen(QColor(255, 112, 112, 245));
    painter.drawText(valueRect, Qt::AlignLeft | Qt::AlignVCenter, QString::number(myHero->hp()));

    painter.restore();
}

//绘制技能2的爆炸效果。
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

// 绘制E键弹幕爆发时的环形特效。
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
        for (int i = 0; i < GameConfig::kBulletWheelProjectileCount; ++i) {
            const qreal radians = (kPi * 2.0 / 12.0) * i + progress * 0.3;
            const QPointF direction(std::cos(radians), std::sin(radians));
            painter.drawLine(burst.center + direction * (radius * 0.28),
                             burst.center + direction * (radius + 20.0 * fade));
        }
    }

    painter.restore();
}

//绘制Boss攻击波和Boss死亡爆炸特效。
void MainWindow::drawDragonEffects(QPainter &painter) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    for (const DragonAttackWave &wave : m_dragonAttackWaves) {
        const qreal progress = std::clamp(wave.elapsed / kDragonAttackWaveDurationMs, 0.0, 1.0);
        const qreal fade = 1.0 - progress;
        const qreal radius = (46.0 + kDragonAttackWaveMaxRadius * progress) * wave.scale;

        painter.save();
        painter.translate(wave.center);
        painter.rotate(wave.rotationSeed + progress * 760.0);
        painter.setPen(Qt::NoPen);

        QRadialGradient outerGlow(QPointF(0.0, 6.0), radius);
        outerGlow.setColorAt(0.0, QColor(255, 240, 188, static_cast<int>(185 * fade)));
        outerGlow.setColorAt(0.33, QColor(255, 152, 72, static_cast<int>(165 * fade)));
        outerGlow.setColorAt(0.72, QColor(255, 88, 34, static_cast<int>(120 * fade)));
        outerGlow.setColorAt(1.0, QColor(255, 60, 20, 0));
        painter.setBrush(outerGlow);
        painter.drawEllipse(QRectF(-radius, -radius * 0.82, radius * 2.0, radius * 1.64));

        for (int ring = 0; ring < 4; ++ring) {
            const qreal ringRatio = static_cast<qreal>(ring) / 3.0;
            const qreal ringRadiusX = radius * (0.76 - ringRatio * 0.14);
            const qreal ringRadiusY = radius * (0.34 - ringRatio * 0.05);
            const qreal ringY = radius * 0.42 - ring * 18.0 - progress * 28.0;
            const qreal offsetX = std::sin(progress * 9.0 + ringRatio * 2.0) * (22.0 - ring * 4.0);

            QRadialGradient ringGradient(QPointF(offsetX, ringY - 4.0), ringRadiusX * 1.1);
            ringGradient.setColorAt(0.0, QColor(255, 250, 215, static_cast<int>(165 * fade)));
            ringGradient.setColorAt(0.4, QColor(255, 176, 88, static_cast<int>(150 * fade)));
            ringGradient.setColorAt(0.8, QColor(255, 96, 44, static_cast<int>(118 * fade)));
            ringGradient.setColorAt(1.0, QColor(255, 76, 30, 0));
            painter.setBrush(ringGradient);
            painter.drawEllipse(QPointF(offsetX, ringY), ringRadiusX, ringRadiusY);
        }

        for (int particle = 0; particle < 14; ++particle) {
            const qreal particleAngle = (kPi * 2.0 / 14.0) * particle
                                        + wave.rotationSeed * kPi / 180.0
                                        + progress * 9.5;
            const qreal risePhase = std::fmod(progress * 1.8 + particle * 0.17, 1.0);
            const qreal taper = 1.0 - risePhase * 0.7;
            const qreal particleX = std::cos(particleAngle) * radius * 0.42 * taper;
            const qreal particleY = radius * 0.48 - risePhase * radius * 1.15;
            const qreal particleRadius = (4.0 + (particle % 3)) * (0.55 + taper * 0.65);

            QRadialGradient particleGradient(QPointF(particleX, particleY), particleRadius * 1.7);
            particleGradient.setColorAt(0.0, QColor(255, 244, 198, static_cast<int>(220 * fade)));
            particleGradient.setColorAt(0.45, QColor(255, 190, 92, static_cast<int>(185 * fade)));
            particleGradient.setColorAt(1.0, QColor(255, 112, 36, 0));
            painter.setBrush(particleGradient);
            painter.drawEllipse(QPointF(particleX, particleY), particleRadius, particleRadius);
        }

        QPen ringPen(QColor(255, 230, 158, static_cast<int>(200 * fade)));
        ringPen.setWidth(6);
        painter.setPen(ringPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(QRectF(-radius * 0.74, -radius * 0.18, radius * 1.48, radius * 0.92));
        painter.restore();
    }

    for (const DragonDeathBurst &burst : m_dragonDeathBursts) {
        const qreal progress = std::clamp(burst.elapsed / kDragonDeathBurstDurationMs, 0.0, 1.0);
        const qreal fade = 1.0 - progress;
        const qreal radius = 70.0 + kDragonDeathBurstMaxRadius * progress;

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 84, 36, static_cast<int>(145 * fade)));
        painter.drawEllipse(burst.center, radius, radius);

        painter.setBrush(QColor(255, 170, 72, static_cast<int>(160 * fade)));
        painter.drawEllipse(burst.center, radius * 0.55, radius * 0.55);

        QPen outerRing(QColor(255, 236, 178, static_cast<int>(230 * fade)));
        outerRing.setWidth(9);
        painter.setPen(outerRing);
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(burst.center, radius * 0.86, radius * 0.86);

        QPen flamePen(QColor(255, 224, 132, static_cast<int>(220 * fade)));
        flamePen.setWidth(6);
        flamePen.setCapStyle(Qt::RoundCap);
        painter.setPen(flamePen);
        for (int i = 0; i < 14; ++i) {
            const qreal radians = (kPi * 2.0 / 14.0) * i + progress * 0.55;
            const QPointF direction(std::cos(radians), std::sin(radians));
            painter.drawLine(burst.center + direction * (radius * 0.28),
                             burst.center + direction * (radius + 42.0 * fade));
        }
    }

    painter.restore();
}

//绘制闪现前后位置之间的拖影和冲击效果。
void MainWindow::drawFlashEffect(QPainter &painter) const
{
    const QPixmap &heroFlashPixmap =
        (myHero != nullptr && myHero->level() >= 4 && !m_heroChangedPixmap.isNull())
            ? m_heroChangedPixmap
            : m_heroIdlePixmap;
    if (!m_flashEffectActive || heroFlashPixmap.isNull()) {
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
                               heroFlashPixmap,
                               QRectF(0.0, 0.0, heroFlashPixmap.width(), heroFlashPixmap.height()));
            painter.restore();
        } else {
            painter.drawPixmap(QRectF(ghostPos.x(), ghostPos.y(), HERO_WIDTH, HERO_HEIGHT),
                               heroFlashPixmap,
                               QRectF(0.0, 0.0, heroFlashPixmap.width(), heroFlashPixmap.height()));
        }
    }

    painter.restore();
}

//绘制技能3的激光束可视化效果。
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

//返回英雄中心点
//方便作为子弹发射点和相机跟随基准。
QPointF MainWindow::heroCenter() const
{
    return QPointF(myHero->Hero_x + HERO_WIDTH / 2.0,
                   myHero->Hero_y + HERO_HEIGHT / 2.0);
}

//根据英雄位置计算相机偏移，让窗口显示英雄附近区域。
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

//返回地图世界宽度
int MainWindow::worldWidth() const
{
    return std::max(GAME_WIDTH, m_mapPixmap.isNull() ? 0 : m_mapPixmap.width());
}
// 返回地图世界高度；若地图未加载则退化为窗口高度。
int MainWindow::worldHeight() const
{
    return std::max(GAME_HEIGHT, m_mapPixmap.isNull() ? 0 : m_mapPixmap.height());
}
