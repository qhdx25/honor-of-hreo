#ifndef BOSS2ENEMY_H
#define BOSS2ENEMY_H

#include "enemy.h"

class Boss2Enemy : public Enemy
{
public:
    explicit Boss2Enemy(const QPointF &startPos);
};

#endif // BOSS2ENEMY_H
