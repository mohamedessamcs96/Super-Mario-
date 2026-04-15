#include "Player.h"

Player::Player(int x, int y)
    : Entity(x, y, 100, "Mario"), lives(3), score(0),
      velocityY(0.0f), onGround(false), dead(false), facingLeft(false) {}

void Player::moveLeft()  { if (!dead) { x--; facingLeft = true;  } }
void Player::moveRight() { if (!dead) { x++; facingLeft = false; } }

void Player::jump() {
    if (!dead && onGround) {
        velocityY = -4.0f;
        onGround  = false;
    }
}

void Player::applyGravity() {
    if (dead) return;
    velocityY += 0.6f;
    if (velocityY >  5.0f) velocityY =  5.0f;
    if (velocityY < -5.0f) velocityY = -5.0f;
    y += static_cast<int>(velocityY);
}

void Player::land(int groundY) {
    y         = groundY;
    velocityY = 0.0f;
    onGround  = true;
}

void Player::loseLife() { if (lives > 0) lives--; dead = true; }

void Player::kill() {
    if (!dead) { dead = true; if (lives > 0) lives--; }
}

void Player::addScore(int pts)  { score += pts; }

void Player::respawn(int sx, int sy) {
    x = sx; y = sy;
    velocityY = 0.0f;
    onGround  = false;
    dead      = false;
}
