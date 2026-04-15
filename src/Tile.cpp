#include "Tile.h"

Tile::Tile(long long x, long long y, Tile::Tiletype type) : x(x), y(y), type(type) {}

Tile::TileEffect Tile::touch() { return Tile::None; }

char Tile::symbol() const {
    switch (type) {
        case Brick:      return '#';
        case Grass:      return 'G';
        case Mud:        return 'm';
        case Steel:      return '=';
        case MysteryBox: return '?';
        case Flag:       return 'F';
        default:         return ' ';
    }
}

DeadlyTile::DeadlyTile(long long x, long long y, Tile::Tiletype type) : Tile(x, y, type) {}
Tile::TileEffect DeadlyTile::touch() { return Tile::Deadly; }

InteractiveTile::InteractiveTile(long long x, long long y, Tile::Tiletype type, PrizeType prize)
    : Tile(x, y, type), prize(prize) {}

Tile::TileEffect InteractiveTile::touch() {
    if (hit == 0) { hit++; return Tile::Prize; }
    return Tile::None;
}

EndTile::EndTile(long long x, long long y, Tile::Tiletype type) : Tile(x, y, type) {}
Tile::TileEffect EndTile::touch() { return Tile::Next_Level; }
