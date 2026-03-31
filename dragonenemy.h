#ifndef DRAGONENEMY_H
#define DRAGONENEMY_H

#include "enemy.h"

class DragonEnemy : public Enemy
{
public:
    explicit DragonEnemy(const QPointF &startPos);
};

#endif // DRAGONENEMY_H
