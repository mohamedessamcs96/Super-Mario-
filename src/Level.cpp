#include "Level.h"
#include <cstring>

Level::Level() : spawnX(2), spawnY(ROWS - 3), flagX(COLS - 3), flagY(ROWS - 3) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            grid[r][c] = nullptr;
}

Level::~Level() {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            delete grid[r][c];
    for (Enemy* e : enemies) delete e;
}

// Helper lambdas used inside buildDefault
static void placeTile(Tile* (&grid)[Level::ROWS][Level::COLS],
                      int r, int c, Tile* t) {
    if (r >= 0 && r < Level::ROWS && c >= 0 && c < Level::COLS) {
        delete grid[r][c];
        grid[r][c] = t;
    }
}

void Level::buildDefault() {
    const int GROUND = ROWS - 1;   // bottom row index

    // ── Ground floor (full width, two rows deep) ──────────────────────────
    for (int c = 0; c < COLS; c++) {
        placeTile(grid, GROUND,     c, new Tile(c, GROUND,     Tile::Grass));
        placeTile(grid, GROUND - 1, c, new Tile(c, GROUND - 1, Tile::Mud));
    }

    // ── Gap in the floor (forces a jump) ─────────────────────────────────
    for (int c = 15; c <= 17; c++) {
        delete grid[GROUND][c];     grid[GROUND][c]     = nullptr;
        delete grid[GROUND-1][c];   grid[GROUND-1][c]   = nullptr;
    }

    // ── Second gap ────────────────────────────────────────────────────────
    for (int c = 30; c <= 33; c++) {
        delete grid[GROUND][c];     grid[GROUND][c]     = nullptr;
        delete grid[GROUND-1][c];   grid[GROUND-1][c]   = nullptr;
    }

    // ── Floating brick platforms ──────────────────────────────────────────
    // Platform 1  (above gap 1)
    for (int c = 10; c <= 13; c++)
        placeTile(grid, GROUND - 4, c, new Tile(c, GROUND - 4, Tile::Brick));

    // Platform 2
    for (int c = 20; c <= 24; c++)
        placeTile(grid, GROUND - 5, c, new Tile(c, GROUND - 5, Tile::Brick));

    // Platform 3 (tall)
    for (int c = 38; c <= 41; c++)
        placeTile(grid, GROUND - 6, c, new Tile(c, GROUND - 6, Tile::Brick));

    // ── Mystery boxes ─────────────────────────────────────────────────────
    placeTile(grid, GROUND - 4, 7,
              new InteractiveTile(7, GROUND - 4, Tile::MysteryBox, Tile::Coins));
    placeTile(grid, GROUND - 6, 22,
              new InteractiveTile(22, GROUND - 6, Tile::MysteryBox, Tile::PowerUp));

    // ── Deadly spikes (DeadlyTile) ────────────────────────────────────────
    placeTile(grid, GROUND - 2, 25, new DeadlyTile(25, GROUND - 2, Tile::Steel));
    placeTile(grid, GROUND - 2, 26, new DeadlyTile(26, GROUND - 2, Tile::Steel));

    // ── Flag / end tile ───────────────────────────────────────────────────
    placeTile(grid, GROUND - 2, flagX,
              new EndTile(flagX, GROUND - 2, Tile::Flag));
    placeTile(grid, GROUND - 1, flagX, new Tile(flagX, GROUND - 1, Tile::Brick));

    // ── Enemies ───────────────────────────────────────────────────────────
    enemies.push_back(new Enemy(20, GROUND - 2, 18, 28));
    enemies.push_back(new Enemy(40, GROUND - 2, 36, 48));
    enemies.push_back(new Enemy(11, GROUND - 6, 10, 13)); // on platform 1
}

Tile* Level::getTile(int row, int col) const {
    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return nullptr;
    return grid[row][col];
}

bool Level::isSolid(int row, int col) const {
    Tile* t = getTile(row, col);
    return (t != nullptr) && (t->getType() != Tile::Empty);
}
