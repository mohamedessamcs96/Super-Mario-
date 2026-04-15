#ifndef ENEMY_H
#define ENEMY_H

#include "Entity.h"

class Enemy : public Entity {
private:
    int direction;  // -1 = left, +1 = right
    bool alive;
    int patrolLeft;
    int patrolRight;

public:
    Enemy(int x, int y, int patrolLeft, int patrolRight);

    void autoMove();
    bool isAlive() const { return alive; }
    void kill() { alive = false; }
};

#endif
