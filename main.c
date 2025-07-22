#include <ncurses.h>
#include <unistd.h>
#include <string.h>

// Ustawienia planszy
#define WIDTH 80
#define HEIGHT 24

// Ustawienia paletek i piłki
#define PADDLE_HEIGHT 5
#define PADDLE_WIDTH 1
#define PADDLE_X_RANGE 6 // Zakres ruchu poziomego paletek
#define BALL_CHAR 'o'

// Struktura dla obiektów gry
typedef struct {
    int x, y;
} GameObject;

void draw_board(WINDOW *win) {
    box(win, 0, 0);
    mvwprintw(win, 0, WIDTH / 2 - 5, " PING PONG ");
}

void draw_object(WINDOW *win, GameObject obj, char c) {
    mvwaddch(win, obj.y, obj.x, c);
}

void draw_paddle(WINDOW *win, GameObject paddle, int color_pair) {
    wattron(win, COLOR_PAIR(color_pair));
    for (int i = 0; i < PADDLE_HEIGHT; ++i) {
        mvwaddch(win, paddle.y + i, paddle.x, ACS_CKBOARD);
    }
    wattroff(win, COLOR_PAIR(color_pair));
}

void show_instructions(WINDOW *win) {
    werase(win);
    box(win, 0, 0);
    
    char *title = "STEROWANIE";
    mvwprintw(win, 2, (WIDTH - strlen(title)) / 2, "%s", title);

    mvwprintw(win, 5, 10, "Gracz 1 (lewa paletka):");
    mvwprintw(win, 7, 12, "W - Gora");
    mvwprintw(win, 8, 12, "S - Dol");
    mvwprintw(win, 9, 12, "A - Lewo");
    mvwprintw(win, 10, 12, "D - Prawo");

    mvwprintw(win, 5, WIDTH - 35, "Gracz 2 (prawa paletka):");
    mvwprintw(win, 7, WIDTH - 33, "Strzalka w gore - Gora");
    mvwprintw(win, 8, WIDTH - 33, "Strzalka w dol  - Dol");
    mvwprintw(win, 9, WIDTH - 33, "Strzalka w lewo - Lewo");
    mvwprintw(win, 10, WIDTH - 33, "Strzalka w prawo- Prawo");

    char *quit_msg = "Q - Wyjscie z gry";
    mvwprintw(win, 15, (WIDTH - strlen(quit_msg)) / 2, "%s", quit_msg);

    char *start_msg = "Nacisnij dowolny klawisz, aby rozpoczac...";
    mvwprintw(win, HEIGHT - 3, (WIDTH - strlen(start_msg)) / 2, "%s", start_msg);

    wrefresh(win);

    // Czekaj na wciśnięcie klawisza (blokująco)
    nodelay(win, FALSE);
    wgetch(win);
    nodelay(win, TRUE); // Wróć do trybu nieblokującego dla gry
}

int main() {
    // Inicjalizacja ncurses
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    
    // Włączenie kolorów
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK); // Paletka 1 (czerwona)
    init_pair(2, COLOR_BLUE, COLOR_BLACK); // Paletka 2 (niebieska)

    // Utworzenie okna gry
    WINDOW *win = newwin(HEIGHT, WIDTH, (LINES - HEIGHT) / 2, (COLS - WIDTH) / 2);
    keypad(win, TRUE);

    // Pokaż instrukcje i czekaj na start
    show_instructions(win);

    // Pozycje startowe i granice
    const int player1_start_x = 2;
    const int player2_start_x = WIDTH - 3;
    GameObject player1 = {player1_start_x, HEIGHT / 2 - PADDLE_HEIGHT / 2};
    GameObject player2 = {player2_start_x, HEIGHT / 2 - PADDLE_HEIGHT / 2};
    GameObject ball = {WIDTH / 2, HEIGHT / 2};
    
    // Kierunek piłki
    int ball_dx = -1;
    int ball_dy = 1;

    int score1 = 0;
    int score2 = 0;

    while (1) {
        // Pobranie ruchu gracza
        int ch = wgetch(win);
        switch (ch) {
            // Gracz 1 (lewa paletka)
            case 'w':
                if (player1.y > 1) player1.y--;
                break;
            case 's':
                if (player1.y < HEIGHT - PADDLE_HEIGHT - 1) player1.y++;
                break;
            case 'a':
                if (player1.x > 1) player1.x--;
                break;
            case 'd':
                if (player1.x < player1_start_x + PADDLE_X_RANGE) player1.x++;
                break;

            // Gracz 2 (prawa paletka)
            case KEY_UP:
                if (player2.y > 1) player2.y--;
                break;
            case KEY_DOWN:
                if (player2.y < HEIGHT - PADDLE_HEIGHT - 1) player2.y++;
                break;
            case KEY_LEFT:
                if (player2.x > player2_start_x - PADDLE_X_RANGE) player2.x--;
                break;
            case KEY_RIGHT:
                if (player2.x < WIDTH - 2) player2.x++;
                break;

            case 'q': // Wyjście z gry
                endwin();
                return 0;
        }

        // Ruch piłki
        ball.x += ball_dx;
        ball.y += ball_dy;

        // Kolizje ze ścianami (góra/dół)
        if (ball.y <= 0 || ball.y >= HEIGHT - 1) {
            ball_dy *= -1;
        }

        // Kolizje z paletkami
        if ((ball.x == player1.x + 1 && ball.y >= player1.y && ball.y < player1.y + PADDLE_HEIGHT) ||
            (ball.x == player2.x - 1 && ball.y >= player2.y && ball.y < player2.y + PADDLE_HEIGHT)) {
            ball_dx *= -1;
        }

        // Punktacja
        if (ball.x < 0) {
            score2++;
            ball.x = WIDTH / 2;
            ball.y = HEIGHT / 2;
        } else if (ball.x > WIDTH) {
            score1++;
            ball.x = WIDTH / 2;
            ball.y = HEIGHT / 2;
        }

        // Rysowanie
        werase(win);
        draw_board(win);
        
        // Wynik
        mvwprintw(win, 1, WIDTH / 2 - 5, "%d | %d", score1, score2);

        draw_paddle(win, player1, 1); // Rysuj paletkę 1 z parą kolorów 1
        draw_paddle(win, player2, 2); // Rysuj paletkę 2 z parą kolorów 2
        draw_object(win, ball, BALL_CHAR);
        
        wrefresh(win);

        // Stała prędkość gry
        usleep(100000);
    }

    // Zakończenie pracy z ncurses
    endwin();
    return 0;
}
