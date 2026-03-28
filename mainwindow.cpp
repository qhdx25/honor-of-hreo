#include "mainwindow.h"

#include "assetpaths.h"
#include "bullet.h"
#include "config.h"
#include "enemy.h"
#include "skill2bullet.h"
#include "skilliconwidget.h"

#include <QIcon>
#include <QKeyEvent>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QPolygonF>
#include <QRandomGenerator>
#include <QTimer>
#include <QBrush>
#include <QString>
#include <algorithm>
#include <cmath>

namespace {
constexpr qreal kPi = 3.14159265358979323846;
constexpr qreal kHalfPi = kPi / 2.0;

QString assetPath(const QString &fileName)
{
    return QString::fromUtf8(kAssetDir) + "/" + fileName;
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
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initScene();

    myHero = new hero();

    m_gameTimer = new QTimer(this);
    m_gameTimer->setInterval(16);
    QObject::connect(m_gameTimer, &QTimer::timeout, this, [this]() { updateBullets(); });
    m_gameTimer->start();

    m_enemyTimer = new QTimer(this);
    m_enemyTimer->setInterval(1200);
    QObject::connect(m_enemyTimer, &QTimer::timeout, this, [this]() { spawnEnemy(); });
    m_enemyTimer->start();

    const QPoint skill1Pos(1580, 840);
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

    const QPoint skill2Pos(1710, 720);
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

    const QPoint skill3Pos(1840, 600);
    m_skill3Icon = new SkillIconWidget(this);
    m_skill3Icon->setFocusPolicy(Qt::NoFocus);
    m_skill3Icon->setFrames(QVector<QString>{assetPath("skill3.png")});
    m_skill3Icon->setGeometry(skill3Pos.x(),
                              skill3Pos.y(),
                              SkillIconWidget::IconSize,
                              SkillIconWidget::IconSize);
}

MainWindow::~MainWindow()
{
    for (Bullet *bullet : m_bullets) {
        delete bullet;
    }
    m_bullets.clear();

    for (Enemy *enemy : m_enemies) {
        delete enemy;
    }
    m_enemies.clear();

    delete myHero;
}

void MainWindow::initScene()
{
    setFixedSize(GAME_WIDTH, GAME_HEIGHT);
    setWindowTitle(GAME_TITLE);
    setWindowIcon(QIcon(assetPath("firsta.jpg")));
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    QPixmap bgPix(assetPath("background2.jpg"));
    painter.drawPixmap(0, 0, width(), height(), bgPix);

    if (myHero != nullptr) {
        QPixmap heroPix(assetPath("test.png"));
        painter.drawPixmap(myHero->Hero_x, myHero->Hero_y, HERO_WIDTH, HERO_HEIGHT, heroPix);
    }

    if (m_skillAiming) {
        drawSkillArrow(painter);
    }

    for (const Bullet *bullet : m_bullets) {
        bullet->paint(painter);
    }

    for (const Enemy *enemy : m_enemies) {
        enemy->paint(painter);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (myHero != nullptr) {
        myHero->updatePos(event->key(), width(), height());
    }

    update();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && myHero != nullptr) {
        m_bullets.push_back(new Bullet(myHero->shootOrigin(), event->pos()));
        update();
    }

    QMainWindow::mousePressEvent(event);
}

void MainWindow::updateBullets()
{
    for (int i = m_bullets.size() - 1; i >= 0; --i) {
        Bullet *bullet = m_bullets.at(i);
        bullet->update();

        if (bullet->isOutOfBounds(width(), height()) || bullet->hasReachedMaxDistance()) {
            delete bullet;
            m_bullets.removeAt(i);
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

    updateEnemies();
    update();
}

void MainWindow::spawnEnemy()
{
    if (myHero == nullptr) {
        return;
    }

    const int typeIndex = static_cast<int>(QRandomGenerator::global()->bounded(5u));
    const Enemy::Type type = static_cast<Enemy::Type>(typeIndex);
    const int spawnX = width() + 40;
    const int spawnY = 70;

    m_enemies.push_back(new Enemy(type, QPointF(spawnX, spawnY)));
}

void MainWindow::updateEnemies()
{
    if (myHero == nullptr) {
        return;
    }

    const QPointF center = heroCenter();

    for (int i = m_enemies.size() - 1; i >= 0; --i) {
        Enemy *enemy = m_enemies.at(i);
        enemy->updateToward(center);
        enemy->updateEnteredState(width(), height());

        const bool shouldRemove =
            enemy->reachesTarget(center) ||
            (enemy->hasEnteredScreen() && enemy->isOutOfBounds(width(), height()));

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
    case SkillType::None:
        break;
    }

    clearSkillAim();
    update();
}

void MainWindow::castSkill1()
{
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
}

void MainWindow::castSkill2()
{
    const QPointF origin = myHero->shootOrigin();
    const QPointF target = origin + m_skillDirection * 480.0;
    m_bullets.push_back(new Skill2Bullet(origin, target, 20.0, 1200.0));
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

QPointF MainWindow::heroCenter() const
{
    return QPointF(myHero->Hero_x + HERO_WIDTH / 2.0,
                   myHero->Hero_y + HERO_HEIGHT / 2.0);
}
