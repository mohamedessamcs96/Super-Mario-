#ifndef PLAYER_H
#define PLAYER_H

#include "Entity.h"

class Player : public Entity {
private:
    int lives;
    int score;
    float velocityY;
    bool onGround;
    bool dead;
    bool facingLeft;

public:
    Player(int x, int y);

    // Movement
    void moveLeft();
    void moveRight();
    void jump();

    // Physics tick — applies gravity and moves vertically
    // Returns new Y after gravity; caller handles tile collision
    void applyGravity();

    // State
    void land(int groundY);      // snap to ground, zero velocity
    void loseLife();
    void addScore(int points);
    void respawn(int spawnX, int spawnY);
    void kill();

    int getLives()  const { return lives; }
    int getScore()  const { return score; }
    float getVelocityY() const { return velocityY; }
    bool isOnGround()   const { return onGround; }
    bool isDead()       const { return dead; }
    bool isFacingLeft() const { return facingLeft; }

    // Let physics system set y directly
    void setY(int newY) { y = newY; }
    void setOnGround(bool g) { onGround = g; }
    void setVelocityY(float v) { velocityY = v; }
};

#endif
