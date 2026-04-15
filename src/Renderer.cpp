#include "Renderer.h"
#include "Enemy.h"
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

// ANSI
#define RS  "\033[0m"
#define B   "\033[1m"
#define RD  "\033[31m"
#define GR  "\033[32m"
#define YL  "\033[33m"
#define MG  "\033[35m"
#define CY  "\033[36m"
#define WH  "\033[37m"
#define bBL "\033[44m"
#define bGR "\033[42m"
#define bYL "\033[43m"

static char buf[131072];
static int  bp;
static void ap(const char* s){ int n=strlen(s); memcpy(buf+bp,s,n); bp+=n; }

static void termSize(int& c, int& r){
    struct winsize w;
    if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&w)==0&&w.ws_col>0){c=w.ws_col;r=w.ws_row;}
    else{c=80;r=24;}
}

void Renderer::draw(const Level& lv, const Player& p, bool over, bool won){
    bp=0;

    int tCols, tRows;
    termSize(tCols, tRows);

    // viewCols: fit in terminal, never exceed level width
    // Each tile = 2 chars, borders = 2, so max tiles = (tCols-2)/2
    int vCols = (tCols - 2) / 2;
    if(vCols > Level::COLS) vCols = Level::COLS;
    if(vCols < 8) vCols = 8;

    // viewRows: HUD + topBorder + grid + botBorder + status = grid + 4
    int vRows = tRows - 4;
    if(vRows > Level::ROWS) vRows = Level::ROWS;
    if(vRows < 4) vRows = 4;

    // Cameras
    int camC = p.getX() - vCols/2;
    if(camC < 0) camC = 0;
    if(camC + vCols > Level::COLS) camC = Level::COLS - vCols;

    int camR = p.getY() - vRows/2;
    if(camR < 0) camR = 0;
    if(camR + vRows > Level::ROWS) camR = Level::ROWS - vRows;

    // Enemy map
    bool em[Level::ROWS][Level::COLS];
    memset(em,0,sizeof(em));
    for(Enemy* e: const_cast<Level&>(lv).getEnemies())
        if(e->isAlive()&&e->getY()>=0&&e->getY()<Level::ROWS
           &&e->getX()>=0&&e->getX()<Level::COLS)
            em[e->getY()][e->getX()]=true;

    // Go to top-left, hide cursor
    ap("\033[H\033[?25l");

    // ── HUD ───────────────────────────────────────────────────────────────────
    ap(B WH "MARIO" RS);
    ap(RD B "  Lives:");
    for(int i=0;i<p.getLives();i++) ap(" (v)");
    for(int i=p.getLives();i<3;i++) ap("    ");
    ap(RS YL B "  Score:");
    char tmp[32]; snprintf(tmp,32,"%05d",p.getScore()); ap(tmp);
    ap(RS CY "  HP:"); snprintf(tmp,32,"%d",p.getHealth()); ap(tmp);
    ap(RS "\033[K\n");

    // ── Top border: exactly 1 + vCols*2 + 1 chars ────────────────────────────
    ap(WH "+");
    for(int c=0;c<vCols;c++) ap("--");
    ap("+" RS "\033[K\n");

    // ── Grid rows ─────────────────────────────────────────────────────────────
    for(int r=0;r<vRows;r++){
        int wr = camR + r;
        ap(WH "|" RS);

        for(int c=0;c<vCols;c++){
            int wc = camC + c;

            // Player
            if(wr==p.getY()&&wc==p.getX()){
                if(p.isDead())          ap(RD B "XX" RS);
                else if(!p.isOnGround())ap(MG B ">^" RS);
                else                    ap(MG B ">)" RS);
                continue;
            }
            // Enemy
            if(wr>=0&&wr<Level::ROWS&&wc>=0&&wc<Level::COLS&&em[wr][wc]){
                ap(RD B "GE" RS); continue;
            }
            // Tile
            if(wr>=0&&wr<Level::ROWS&&wc>=0&&wc<Level::COLS){
                Tile* t=lv.getTile(wr,wc);
                if(t) switch(t->symbol()){
                    case '#': ap(YL "##" RS);     break;
                    case 'G': ap(bGR GR "==" RS); break;
                    case 'm': ap(YL "~~" RS);      break;
                    case '=': ap(WH "[]" RS);      break;
                    case '?': ap(bYL B "??" RS);   break;
                    case '[': ap(YL "[]" RS);       break;
                    case 'F': ap(GR B "F!" RS);    break;
                    case '^': ap(RD "/\\" RS);      break;
                    default:  ap(bBL "  " RS);     break;
                } else ap(bBL "  " RS);
            } else ap(bBL "  " RS);
        }
        ap(WH "|" RS "\033[K\n");
    }

    // ── Bottom border ─────────────────────────────────────────────────────────
    ap(WH "+");
    for(int c=0;c<vCols;c++) ap("--");
    ap("+" RS "\033[K\n");

    // ── Status ────────────────────────────────────────────────────────────────
    if(over)
        ap(RD B "GAME OVER  any key=restart  Q=quit" RS "\033[K\n");
    else if(won)
        ap(GR B "YOU WIN! :D  Q=quit" RS "\033[K\n");
    else if(p.isDead())
        ap(RD "Died! Respawning..." RS "\033[K\n");
    else
        ap(CY "[A]Left [D]Right [W/Space]Jump" RS "\033[K\n");

    // Clear everything below
    ap("\033[J");

    fwrite(buf,1,bp,stdout);
    fflush(stdout);
}
