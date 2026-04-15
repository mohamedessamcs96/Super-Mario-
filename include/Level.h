#ifndef LEVEL_H
#define LEVEL_H

#include <vector>
#include <utility>
#include "Tile.h"
#include "Enemy.h"

class Level {
public:
    static const int COLS = 60;   // world width in tiles
    static const int ROWS = 20;   // world height in tiles

private:
    // Owned 2-D grid: grid[row][col]
    Tile* grid[ROWS][COLS];

    int spawnX, spawnY;   // tile coords
    int flagX,  flagY;

    std::vector<Enemy*> enemies;

public:
    Level();
    ~Level();

    // Build the default level layout
    void buildDefault();

    // Tile access — returns nullptr for Empty/out-of-bounds
    Tile* getTile(int row, int col) const;

    // Solid tile check (blocks movement)
    bool isSolid(int row, int col) const;

    std::pair<int,int> getSpawn() const { return {spawnX, spawnY}; }
    std::pair<int,int> getFlag()  const { return {flagX,  flagY};  }

    std::vector<Enemy*>& getEnemies() { return enemies; }
};

#endif
