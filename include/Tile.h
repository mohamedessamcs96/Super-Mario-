#ifndef TILE_H
#define TILE_H

class Tile {
public:
    enum Tiletype {
        Brick,
        Grass,
        Mud,
        Steel,
        MysteryBox,
        Flag,
        Empty,
    };

    enum TileEffect {
        None,
        Deadly,
        Prize,
        Next_Level,
    };

    enum PrizeType {
        Coins,
        PowerUp,
    };

protected:
    long long x, y;
    Tiletype type;

public:
    Tile(long long x, long long y, Tiletype type);
    virtual ~Tile() {}

    long long getX() const { return x; }
    long long getY() const { return y; }
    Tiletype getType() const { return type; }

    virtual TileEffect touch();

    // Returns the display character for this tile
    virtual char symbol() const;
};

class DeadlyTile : public Tile {
public:
    DeadlyTile(long long x, long long y, Tiletype type);
    TileEffect touch() override;
    char symbol() const override { return '^'; }
};

class InteractiveTile : public Tile {
private:
    int hit = 0;
    PrizeType prize;

public:
    InteractiveTile(long long x, long long y, Tiletype type, PrizeType prize);
    TileEffect touch() override;
    PrizeType getPrizeType() const { return prize; }
    char symbol() const override { return hit > 0 ? '[' : '?'; }
};

class EndTile : public Tile {
public:
    EndTile(long long x, long long y, Tiletype type);
    TileEffect touch() override;
    char symbol() const override { return 'F'; }
};

#endif
