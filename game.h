#ifndef GAME_H
#define GAME_H

#include <ncurses.h>
#include <unistd.h>
#include <string.h>

// Ustawienia planszy
#define WIDTH 80
#define HEIGHT 24

// Ustawienia paletek i piłki
#define PADDLE_HEIGHT 5
#define BALL_CHAR 'o'

// Struktura dla obiektów gry
typedef struct {
    int x, y;
} GameObject;

// Struktura stanu gry do przesyłania przez sieć
typedef struct {
    int ball_x, ball_y;
    int player1_x, player1_y;
    int player2_x, player2_y;
    int score1, score2;
} GameState;

// --- Funkcje rysujące (wspólne dla klienta i serwera) ---

// Używamy `static inline`, aby uniknąć błędów linkera, gdy ten plik
// będzie dołączony do wielu plików .c

static inline void draw_board(WINDOW *win) {
    box(win, 0, 0);
    mvwprintw(win, 0, WIDTH / 2 - 5, " PING PONG ");
}

static inline void draw_paddle(WINDOW *win, int x, int y, int color_pair) {
    wattron(win, COLOR_PAIR(color_pair));
    for (int i = 0; i < PADDLE_HEIGHT; ++i) {
        mvwaddch(win, y + i, x, ACS_CKBOARD);
    }
    wattroff(win, COLOR_PAIR(color_pair));
}

static inline void draw_ball(WINDOW *win, int x, int y) {
    mvwaddch(win, y, x, BALL_CHAR);
}

#endif // GAME_H
