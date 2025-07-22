#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "game.h"

#define PORT 8888

void die(char *s) {
    perror(s);
    exit(1);
}

int main() {
    int listen_sock, conn_sock;
    struct sockaddr_in server, client;
    socklen_t slen = sizeof(client);

    // --- Konfiguracja sieci ---
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        die("socket");
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr*)&server, sizeof(server)) == -1) {
        die("bind");
    }
    listen(listen_sock, 1);
    printf("Czekam na polaczenie na porcie %d...\n", PORT);

    if ((conn_sock = accept(listen_sock, (struct sockaddr*)&client, &slen)) == -1) {
        die("accept");
    }
    printf("Polaczono z %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

    // --- Inicjalizacja ncurses ---
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_BLUE, COLOR_BLACK);

    WINDOW *win = newwin(HEIGHT, WIDTH, (LINES - HEIGHT) / 2, (COLS - WIDTH) / 2);
    keypad(win, TRUE);
    nodelay(win, TRUE);

    // Stan gry
    GameState state;
    state.player1_x = 2;
    state.player1_y = HEIGHT / 2 - PADDLE_HEIGHT / 2;
    state.player2_x = WIDTH - 3;
    state.player2_y = HEIGHT / 2 - PADDLE_HEIGHT / 2;
    state.ball_x = WIDTH / 2;
    state.ball_y = HEIGHT / 2;
    state.score1 = 0;
    state.score2 = 0;

    int ball_dx = -1;
    int ball_dy = 1;

    // --- Pętla gry (logika serwera) ---
    while (1) {
        // Odbierz ruch od klienta (Gracz 2)
        int player2_move = 0;
        recv(conn_sock, &player2_move, sizeof(player2_move), 0);
        switch (ntohl(player2_move)) {
            case KEY_UP: if (state.player2_y > 1) state.player2_y--; break;
            case KEY_DOWN: if (state.player2_y < HEIGHT - PADDLE_HEIGHT - 1) state.player2_y++; break;
        }

        // Sprawdź ruch lokalny (Gracz 1)
        int ch = wgetch(win);
        switch (ch) {
            case 'w': if (state.player1_y > 1) state.player1_y--; break;
            case 's': if (state.player1_y < HEIGHT - PADDLE_HEIGHT - 1) state.player1_y++; break;
            case 'q': close(conn_sock); close(listen_sock); endwin(); return 0;
        }

        // Aktualizacja pozycji piłki
        state.ball_x += ball_dx;
        state.ball_y += ball_dy;

        // Kolizje ze ścianami
        if (state.ball_y <= 0 || state.ball_y >= HEIGHT - 1) {
            ball_dy *= -1;
        }

        // Kolizje z paletkami
        if ((state.ball_x == state.player1_x + 1 && state.ball_y >= state.player1_y && state.ball_y < state.player1_y + PADDLE_HEIGHT) ||
            (state.ball_x == state.player2_x - 1 && state.ball_y >= state.player2_y && state.ball_y < state.player2_y + PADDLE_HEIGHT)) {
            ball_dx *= -1;
        }

        // Punktacja
        if (state.ball_x < 0) {
            state.score2++;
            state.ball_x = WIDTH / 2;
            state.ball_y = HEIGHT / 2;
        } else if (state.ball_x > WIDTH) {
            state.score1++;
            state.ball_x = WIDTH / 2;
            state.ball_y = HEIGHT / 2;
        }

        // Wyślij stan gry do klienta
        send(conn_sock, &state, sizeof(GameState), 0);

        // Rysowanie
        werase(win);
        draw_board(win);
        mvwprintw(win, 1, WIDTH / 2 - 5, "%d | %d", state.score1, state.score2);
        draw_paddle(win, state.player1_x, state.player1_y, 1);
        draw_paddle(win, state.player2_x, state.player2_y, 2);
        draw_ball(win, state.ball_x, state.ball_y);
        wrefresh(win);

        usleep(50000); // Krótszy sen dla płynniejszej gry sieciowej
    }

    close(conn_sock);
    close(listen_sock);
    endwin();
    return 0;
}
