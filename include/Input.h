#ifndef INPUT_H
#define INPUT_H

// Sets terminal to raw mode (non-blocking, no echo).
// Call once at startup; restoreTerminal() at exit.
void initTerminal();
void restoreTerminal();

// Returns the key pressed, or 0 if nothing is waiting.
// Arrow keys return: 'A'(up) 'B'(down) 'C'(right) 'D'(left) via escape seq.
char getKey();

#endif
