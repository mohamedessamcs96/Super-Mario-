#ifndef RENDERER_H
#define RENDERER_H

#include "Level.h"
#include "Player.h"

// VIEW_COLS tiles shown horizontally. Each tile = 2 chars.
// Total row width = 2 + VIEW_COLS*2 + 2 = must be <= terminal width (80)
// So VIEW_COLS <= 38  (2 + 76 + 2 = 80)
static const int VIEW_COLS = 38;

// VIEW_ROWS must equal Level::ROWS so we render the entire world height
static const int VIEW_ROWS = 20;   // matches Level::ROWS exactly

class Renderer {
public:
    static void draw(const Level& level, const Player& player, bool gameOver, bool won);
};

#endif
