#include "Entity.h"

Entity::Entity(int x, int y, int health, std::string name)
    : x(x), y(y), health(health), name(name) {}

void Entity::move(int dx, int dy) { x += dx; y += dy; }

void Entity::takeDamage(int amount) {
    health -= amount;
    if (health < 0) health = 0;
}

int Entity::getX() const { return x; }
int Entity::getY() const { return y; }
int Entity::getHealth() const { return health; }
std::string Entity::getName() const { return name; }
