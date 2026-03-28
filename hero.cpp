#include "hero.h"
#include "config.h"

#include <QKeyEvent>

hero::hero()
{
    Hero_x = 100;
    Hero_y = 100;
    Hero_speed = 10;
}

void hero::shoot()
{
}

QPointF hero::shootOrigin() const
{
    return QPointF(Hero_x + 48, Hero_y + 36);
}

void hero::updatePos(int key, int gameWidth, int gameHeight)
{
    if (key == Qt::Key_W) {
        Hero_y -= Hero_speed;
    } else if (key == Qt::Key_S) {
        Hero_y += Hero_speed;
    } else if (key == Qt::Key_A) {
        Hero_x -= Hero_speed;
    } else if (key == Qt::Key_D) {
        Hero_x += Hero_speed;
    }

    if (Hero_x < 0) {
        Hero_x = 0;
    } else if (Hero_x > gameWidth - HERO_WIDTH) {
        Hero_x = gameWidth - HERO_WIDTH;
    }

    if (Hero_y < 0) {
        Hero_y = 0;
    } else if (Hero_y > gameHeight - HERO_HEIGHT) {
        Hero_y = gameHeight - HERO_HEIGHT;
    }
}
