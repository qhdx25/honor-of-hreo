#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPoint>
#include <QPointF>
#include <QTimer>
#include <QVector>

#include "hero.h"

class SkillIconWidget;
class Bullet;
class Enemy;
class QPainter;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initScene();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    enum class SkillType {
        None,
        Skill1,
        Skill2,
        Skill3
    };

    void spawnEnemy();
    void updateBullets();
    void updateEnemies();
    void beginSkillAim(SkillType skill);
    void updateSkillAim(SkillType skill, const QPoint &dragOffset);
    void releaseSkill(SkillType skill, const QPoint &dragOffset);
    void clearSkillAim();
    void castSkill1();
    void castSkill2();
    void drawSkillArrow(QPainter &painter) const;
    QPointF heroCenter() const;

    SkillIconWidget *m_skill1Icon = nullptr;
    SkillIconWidget *m_skill2Icon = nullptr;
    SkillIconWidget *m_skill3Icon = nullptr;
    QTimer *m_gameTimer = nullptr;
    QTimer *m_enemyTimer = nullptr;
    hero *myHero = nullptr;
    QVector<Bullet *> m_bullets;
    QVector<Enemy *> m_enemies;
    SkillType m_activeSkill = SkillType::None;
    bool m_skillAiming = false;
    QPointF m_skillDirection = QPointF(1.0, 0.0);
    qreal m_skillDragLength = 0.0;
};

#endif // MAINWINDOW_H
