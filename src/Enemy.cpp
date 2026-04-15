#include "Enemy.h"

Enemy::Enemy(int x, int y, int patrolLeft, int patrolRight)
    : Entity(x, y, 50, "Enemy"), direction(1), alive(true),
      patrolLeft(patrolLeft), patrolRight(patrolRight) {}

void Enemy::autoMove() {
    if (!alive) return;
    x += direction;
    if (x <= patrolLeft || x >= patrolRight)
        direction *= -1;
}
