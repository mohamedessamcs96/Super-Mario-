#ifndef ENTITY_H
#define ENTITY_H

#include <string>

class Entity {
protected:
    int x, y;
    int health;
    std::string name;

public:
    Entity(int x, int y, int health, std::string name);

    virtual void move(int dx, int dy);
    virtual void takeDamage(int amount);

    int getX() const;
    int getY() const;
    int getHealth() const;
    std::string getName() const;

    virtual ~Entity() {}
};

#endif
