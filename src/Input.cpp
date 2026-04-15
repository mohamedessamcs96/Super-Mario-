#include "Input.h"
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <cstring>

static struct termios orig;

void initTerminal() {
    tcgetattr(STDIN_FILENO, &orig);
    struct termios raw = orig;
    cfmakeraw(&raw);          // sets all raw flags in one call
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
    fputs("\033[?25h", stdout);
    fflush(stdout);
}

// Returns 0=none, 'a'=left, 'd'=right, 'w'=jump, 'q'=quit
char getKey() {
    char buf[8];
    int n = (int)read(STDIN_FILENO, buf, sizeof(buf));
    if (n <= 0) return 0;

    // ESC sequence (arrow keys)
    if (n >= 3 && buf[0] == '\033' && buf[1] == '[') {
        switch (buf[2]) {
            case 'A': return 'w';  // up    → jump
            case 'C': return 'd';  // right → right
            case 'D': return 'a';  // left  → left
            default:  return 0;
        }
    }
    // Regular key - lowercase it
    char c = buf[0];
    if (c >= 'A' && c <= 'Z') c += 32;
    return c;
}
