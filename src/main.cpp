#include <ncurses.h>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <csignal>
#include <cstring>

#include "Player.h"
#include "Level.h"

using Clock = std::chrono::steady_clock;
using Ms    = std::chrono::milliseconds;

// ── Color pairs ───────────────────────────────────────────────────────────────
#define C_TITLE   1   // bright yellow on black
#define C_MARIO   2   // bright magenta on black
#define C_ENEMY   3   // bright red on black
#define C_GROUND  4   // black on green
#define C_BRICK   5   // bright yellow on black
#define C_BOX     6   // black on yellow
#define C_SPIKE   7   // bright red on black
#define C_FLAG    8   // bright green on black
#define C_SKY     9   // black on blue
#define C_HUD     10  // bright white on black
#define C_DEAD    11  // bright red on black
#define C_MUD     12  // yellow on black
#define C_BORDER  13  // white on black
#define C_INFO    14  // cyan on black

static void cleanup() { endwin(); }
static void onSig(int) { cleanup(); std::exit(0); }

// ── Physics ───────────────────────────────────────────────────────────────────
static bool solid(const Level& lv, int r, int c) {
    if (r<0||r>=Level::ROWS||c<0||c>=Level::COLS) return false;
    return lv.isSolid(r,c);
}
static void hMove(Player& p, const Level& lv, int dx) {
    int nx=p.getX()+dx;
    if (nx<0||nx>=Level::COLS||solid(lv,p.getY(),nx)) return;
    if (dx<0) p.moveLeft(); else p.moveRight();
}
static void vPhysics(Player& p, const Level& lv) {
    if (p.isDead()) return;
    float vy=p.getVelocityY()+0.3f; if(vy>4)vy=4; p.setVelocityY(vy);
    int dy=(vy>=0)?1:-1, ny=p.getY()+dy;
    if (dy>0) {
        if (ny>=Level::ROWS){p.kill();return;}
        if (solid(lv,ny,p.getX())) p.land(p.getY());
        else {p.setY(ny); p.setOnGround(false);}
    } else {
        if (ny<0){p.setVelocityY(0);return;}
        if (solid(lv,ny,p.getX())) {
            Tile*t=lv.getTile(ny,p.getX()); if(t)t->touch();
            p.setVelocityY(1.f);
        } else p.setY(ny);
    }
    if (!solid(lv,p.getY()+1,p.getX())) p.setOnGround(false);
}
static void tileEffects(Player& p, Level& lv) {
    Tile*t=lv.getTile(p.getY(),p.getX()); if(!t)return;
    switch(t->touch()) {
        case Tile::Deadly: if(!p.isDead())p.kill(); break;
        case Tile::Prize:  p.addScore(100); break;
        default: break;
    }
}
static void updateEnemies(Level& lv, Player& p) {
    for (Enemy*e:lv.getEnemies()) {
        if (!e->isAlive()) continue;
        e->autoMove();
        int er=e->getY();
        while(er+1<Level::ROWS&&!solid(lv,er+1,e->getX())) er++;
        if (er!=e->getY()) static_cast<Entity*>(e)->move(0,er-e->getY());
        if (e->getX()==p.getX()&&e->getY()==p.getY()) {
            if (p.getVelocityY()>0){e->kill();p.addScore(200);p.setVelocityY(-2.5f);}
            else if (!p.isDead()) p.kill();
        }
    }
}

// ── Draw tile cell ────────────────────────────────────────────────────────────
static void drawCell(int sy, int sx, char sym) {
    switch(sym) {
        case 'G': attron(COLOR_PAIR(C_GROUND)|A_BOLD);  mvprintw(sy,sx,"=="); break;
        case 'm': attron(COLOR_PAIR(C_MUD)|A_BOLD);     mvprintw(sy,sx,"~~"); break;
        case '#': attron(COLOR_PAIR(C_BRICK)|A_BOLD);   mvprintw(sy,sx,"##"); break;
        case '?': attron(COLOR_PAIR(C_BOX)|A_BOLD);     mvprintw(sy,sx,"??"); break;
        case '[': attron(COLOR_PAIR(C_BRICK));           mvprintw(sy,sx,"[]"); break;
        case '^': attron(COLOR_PAIR(C_SPIKE)|A_BOLD);   mvprintw(sy,sx,"/\\");break;
        case 'F': attron(COLOR_PAIR(C_FLAG)|A_BOLD);    mvprintw(sy,sx,"F!"); break;
        default:  attron(COLOR_PAIR(C_SKY));             mvprintw(sy,sx,"  "); break;
    }
    attrset(A_NORMAL); attron(COLOR_PAIR(C_BORDER));
    attron(COLOR_PAIR(C_BORDER)); // reset
}

// ── Welcome screen ────────────────────────────────────────────────────────────
static void showWelcome() {
    clear();

    // Big title box
    attron(COLOR_PAIR(C_TITLE)|A_BOLD);
    mvprintw(1,2,"+=============================================+");
    mvprintw(2,2,"|                                             |");
    mvprintw(3,2,"|      M A R I O   T E R M I N A L          |");
    mvprintw(4,2,"|                                             |");
    mvprintw(5,2,"+=============================================+");
    attroff(COLOR_PAIR(C_TITLE)|A_BOLD);

    int r=7;
    attron(COLOR_PAIR(C_INFO)|A_BOLD); mvprintw(r++,2,"CONTROLS:"); attroff(COLOR_PAIR(C_INFO)|A_BOLD);
    attron(COLOR_PAIR(C_HUD));
    mvprintw(r++,4,"Arrow keys / A D   Move left & right");
    mvprintw(r++,4,"W / Space / Up     Jump");
    mvprintw(r++,4,"Q                  Quit");
    attroff(COLOR_PAIR(C_HUD));

    r++;
    attron(COLOR_PAIR(C_INFO)|A_BOLD); mvprintw(r++,2,"LEGEND:"); attroff(COLOR_PAIR(C_INFO)|A_BOLD);

    // Each legend item: colored symbol + white description
    attron(COLOR_PAIR(C_MARIO)|A_BOLD);  mvprintw(r,4,">)"); attroff(COLOR_PAIR(C_MARIO)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"You (standing)"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_MARIO)|A_BOLD);  mvprintw(r,4,">^"); attroff(COLOR_PAIR(C_MARIO)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"You (jumping)"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_ENEMY)|A_BOLD);  mvprintw(r,4,"GE"); attroff(COLOR_PAIR(C_ENEMY)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Enemy - jump ON TOP to kill!"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_BOX)|A_BOLD);    mvprintw(r,4,"??"); attroff(COLOR_PAIR(C_BOX)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Mystery box - jump into = +100 pts!"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_SPIKE)|A_BOLD);  mvprintw(r,4,"/\\"); attroff(COLOR_PAIR(C_SPIKE)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Spike - instant death!"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_FLAG)|A_BOLD);   mvprintw(r,4,"F!"); attroff(COLOR_PAIR(C_FLAG)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Flag - reach it to WIN!"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_GROUND)|A_BOLD); mvprintw(r,4,"=="); attroff(COLOR_PAIR(C_GROUND)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Ground"); attroff(COLOR_PAIR(C_HUD));

    attron(COLOR_PAIR(C_BRICK)|A_BOLD);  mvprintw(r,4,"##"); attroff(COLOR_PAIR(C_BRICK)|A_BOLD);
    attron(COLOR_PAIR(C_HUD)); mvprintw(r++,7,"Brick platform"); attroff(COLOR_PAIR(C_HUD));

    r++;
    attron(COLOR_PAIR(C_FLAG)|A_BOLD);
    mvprintw(r++,2,">> Press any key to start! <<");
    attroff(COLOR_PAIR(C_FLAG)|A_BOLD);

    refresh();

    // Wait for key (blocking)
    nodelay(stdscr,FALSE);
    int ch=getch();
    nodelay(stdscr,TRUE);
    if (ch=='q'||ch=='Q') { cleanup(); std::exit(0); }
}

// ── Render game ───────────────────────────────────────────────────────────────
static void renderGame(const Level& lv, const Player& p,
                       bool over, bool won, int vCols, int vRows) {
    int camC=p.getX()-vCols/2;
    if (camC<0) camC=0;
    if (camC+vCols>Level::COLS) camC=Level::COLS-vCols;

    int camR=p.getY()-vRows/2;
    if (camR<0) camR=0;
    if (camR+vRows>Level::ROWS) camR=Level::ROWS-vRows;

    bool em[Level::ROWS][Level::COLS]={};
    for (Enemy*e:const_cast<Level&>(lv).getEnemies())
        if (e->isAlive()&&e->getY()>=0&&e->getY()<Level::ROWS
            &&e->getX()>=0&&e->getX()<Level::COLS)
            em[e->getY()][e->getX()]=true;

    erase(); // faster than clear(), no flicker

    // ── HUD ───────────────────────────────────────────────────────────────────
    attron(COLOR_PAIR(C_TITLE)|A_BOLD); mvprintw(0,0,"MARIO"); attroff(COLOR_PAIR(C_TITLE)|A_BOLD);
    attron(COLOR_PAIR(C_ENEMY)|A_BOLD); printw("  Lives:"); attroff(COLOR_PAIR(C_ENEMY)|A_BOLD);
    attron(COLOR_PAIR(C_ENEMY));
    for (int i=0;i<p.getLives();i++) printw(" <3");
    for (int i=p.getLives();i<3;i++) { attron(A_DIM); printw(" --"); attroff(A_DIM); }
    attroff(COLOR_PAIR(C_ENEMY));
    attron(COLOR_PAIR(C_BOX)|A_BOLD); printw("  Score:%05d", p.getScore()); attroff(COLOR_PAIR(C_BOX)|A_BOLD);
    attron(COLOR_PAIR(C_FLAG)|A_BOLD); printw("  HP:%d", p.getHealth()); attroff(COLOR_PAIR(C_FLAG)|A_BOLD);

    // ── Top border ────────────────────────────────────────────────────────────
    attron(COLOR_PAIR(C_BORDER)|A_BOLD);
    mvprintw(1,0,"+");
    for (int c=0;c<vCols;c++) printw("--");
    printw("+");
    attroff(COLOR_PAIR(C_BORDER)|A_BOLD);

    // ── Grid ──────────────────────────────────────────────────────────────────
    for (int r=0;r<vRows;r++) {
        int wr=camR+r;
        attron(COLOR_PAIR(C_BORDER)|A_BOLD);
        mvprintw(2+r,0,"|");

        for (int c=0;c<vCols;c++) {
            int wc=camC+c;
            int sx=1+c*2, sy=2+r;

            if (wr==p.getY()&&wc==p.getX()) {
                if (p.isDead()) {
                    attron(COLOR_PAIR(C_DEAD)|A_BOLD); mvprintw(sy,sx,"XX"); attroff(COLOR_PAIR(C_DEAD)|A_BOLD);
                } else if (!p.isOnGround()) {
                    attron(COLOR_PAIR(C_MARIO)|A_BOLD); mvprintw(sy,sx,">^"); attroff(COLOR_PAIR(C_MARIO)|A_BOLD);
                } else {
                    attron(COLOR_PAIR(C_MARIO)|A_BOLD); mvprintw(sy,sx,">)"); attroff(COLOR_PAIR(C_MARIO)|A_BOLD);
                }
                continue;
            }
            if (wr>=0&&wr<Level::ROWS&&wc>=0&&wc<Level::COLS&&em[wr][wc]) {
                attron(COLOR_PAIR(C_ENEMY)|A_BOLD); mvprintw(sy,sx,"GE"); attroff(COLOR_PAIR(C_ENEMY)|A_BOLD);
                continue;
            }
            if (wr>=0&&wr<Level::ROWS&&wc>=0&&wc<Level::COLS) {
                Tile*t=lv.getTile(wr,wc);
                drawCell(sy,sx,t?t->symbol():' ');
            } else drawCell(sy,sx,' ');
        }
        attron(COLOR_PAIR(C_BORDER)|A_BOLD);
        mvprintw(2+r,1+vCols*2,"|");
        attroff(COLOR_PAIR(C_BORDER)|A_BOLD);
    }

    // ── Bottom border ─────────────────────────────────────────────────────────
    int bot=2+vRows;
    attron(COLOR_PAIR(C_BORDER)|A_BOLD);
    mvprintw(bot,0,"+");
    for (int c=0;c<vCols;c++) printw("--");
    printw("+");
    attroff(COLOR_PAIR(C_BORDER)|A_BOLD);

    // ── Status line ───────────────────────────────────────────────────────────
    int stat=bot+1;
    if (over) {
        attron(COLOR_PAIR(C_DEAD)|A_BOLD);
        mvprintw(stat,0,"GAME OVER  press any key to restart  |  Q = quit");
        attroff(COLOR_PAIR(C_DEAD)|A_BOLD);
    } else if (won) {
        attron(COLOR_PAIR(C_FLAG)|A_BOLD);
        mvprintw(stat,0,"YOU WIN! :D   Final Score: %d   |  Q = quit", p.getScore());
        attroff(COLOR_PAIR(C_FLAG)|A_BOLD);
    } else if (p.isDead()) {
        attron(COLOR_PAIR(C_DEAD)|A_BOLD);
        mvprintw(stat,0,"You died! Respawning...");
        attroff(COLOR_PAIR(C_DEAD)|A_BOLD);
    } else {
        attron(COLOR_PAIR(C_INFO));
        mvprintw(stat,0,"[A/<] Left   [D/>] Right   [W/Space] Jump   [Q] Quit");
        attroff(COLOR_PAIR(C_INFO));
    }

    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
int main() {
    std::signal(SIGINT, onSig);

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr,TRUE);
    nodelay(stdscr,TRUE);

    start_color();
    use_default_colors();

    // All text pairs use -1 (transparent/black) as background
    // so text is always readable
    init_pair(C_TITLE,  COLOR_YELLOW,  -1);
    init_pair(C_MARIO,  COLOR_MAGENTA, -1);
    init_pair(C_ENEMY,  COLOR_RED,     -1);
    init_pair(C_GROUND, COLOR_BLACK,   COLOR_GREEN);   // black on green = readable
    init_pair(C_BRICK,  COLOR_YELLOW,  -1);
    init_pair(C_BOX,    COLOR_BLACK,   COLOR_YELLOW);  // black on yellow = readable
    init_pair(C_SPIKE,  COLOR_RED,     -1);
    init_pair(C_FLAG,   COLOR_GREEN,   -1);
    init_pair(C_SKY,    COLOR_BLACK,   COLOR_BLUE);
    init_pair(C_HUD,    COLOR_WHITE,   -1);
    init_pair(C_DEAD,   COLOR_RED,     -1);
    init_pair(C_MUD,    COLOR_YELLOW,  -1);
    init_pair(C_BORDER, COLOR_WHITE,   -1);
    init_pair(C_INFO,   COLOR_CYAN,    -1);

    while (true) {
        showWelcome();

        int tRows, tCols;
        getmaxyx(stdscr,tRows,tCols);
        int vCols=tCols/2-1;
        if (vCols>Level::COLS) vCols=Level::COLS;
        if (vCols<8) vCols=8;
        int vRows=tRows-4;
        if (vRows>Level::ROWS) vRows=Level::ROWS;
        if (vRows<4) vRows=4;

        Level lv; lv.buildDefault();
        auto [sx,sy]=lv.getSpawn();
        Player p(sx,sy);

        bool over=false, won=false;
        int  respawn=0, frame=0;

        // Key hold state
        bool holdLeft=false, holdRight=false;

        // We run the game loop at 30fps (33ms).
        // Each frame we poll getch() for ALL pending keys.
        // On key press: set hold flag. Between frames with no key: clear hold flags.
        // This simulates key-release via timeout.
        int  noKeyFrames=0;  // frames since last keypress

        while (true) {
            auto t0=Clock::now(); frame++;

            // ── Drain all pending keys this frame ──────────────────────────
            bool gotKey=false;
            bool wantJump=false;

            int ch;
            while ((ch=getch())!=ERR) {
                gotKey=true;
                if (ch=='q'||ch=='Q') { cleanup(); return 0; }
                if (!over&&!won&&!p.isDead()) {
                    if (ch==KEY_LEFT ||ch=='a') { holdLeft=true;  holdRight=false; }
                    if (ch==KEY_RIGHT||ch=='d') { holdRight=true; holdLeft=false;  }
                    if (ch==KEY_UP||ch=='w'||ch==' ') wantJump=true;
                }
            }

            // If no key pressed for 2 frames → stop moving
            if (!gotKey) {
                noKeyFrames++;
                if (noKeyFrames>=2) { holdLeft=false; holdRight=false; }
            } else {
                noKeyFrames=0;
            }

            if (!over&&!won&&!p.isDead()) {
                if (wantJump) p.jump();
                if (holdLeft)       hMove(p,lv,-1);
                else if (holdRight) hMove(p,lv,1);
            }

            if (!over&&!won&&frame%2==0) vPhysics(p,lv);
            if (!over&&!won&&!p.isDead()) {
                tileEffects(p,lv);
                if (frame%2==0) updateEnemies(lv,p);
                auto [fx,fy]=lv.getFlag();
                if (p.getX()==fx&&p.getY()<=fy) won=true;
            }
            if (!over&&!won&&p.isDead()) {
                if (++respawn>=40) {
                    respawn=0; holdLeft=holdRight=false;
                    if (p.getLives()<=0) over=true;
                    else { auto [rx,ry]=lv.getSpawn(); p.respawn(rx,ry); }
                }
            }

            renderGame(lv,p,over,won,vCols,vRows);

            if (over||won) {
                nodelay(stdscr,FALSE);
                int c=getch();
                nodelay(stdscr,TRUE);
                if (c=='q'||c=='Q') { cleanup(); return 0; }
                break;
            }

            auto elapsed=Clock::now()-t0;
            if (elapsed<Ms(33)) std::this_thread::sleep_for(Ms(33)-elapsed);
        }
    }

    cleanup(); return 0;
}
