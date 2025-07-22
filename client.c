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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uzycie: %s <adres IP serwera>\n", argv[0]);
        exit(1);
    }

    int sock;
    struct sockaddr_in server;

    // --- Konfiguracja sieci ---
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        die("socket");
    }
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
        die("connect");
    }
    printf("Polaczono z serwerem.\n");

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

    GameState state;

    // --- Pętla klienta ---
    while (1) {
        // Sprawdź ruch lokalnego gracza i wyślij do serwera
        int ch = wgetch(win);
        int move = 0;
        if (ch == KEY_UP || ch == KEY_DOWN) {
            move = htonl(ch);
        }
        send(sock, &move, sizeof(move), 0);

        if (ch == 'q') {
            break;
        }

        // Odbierz stan gry z serwera
        if (recv(sock, &state, sizeof(GameState), 0) < 0) {
            break; // Serwer zamknął połączenie
        }

        // Rysowanie na podstawie otrzymanego stanu
        werase(win);
        draw_board(win);
        mvwprintw(win, 1, WIDTH / 2 - 5, "%d | %d", state.score1, state.score2);
        draw_paddle(win, state.player1_x, state.player1_y, 1);
        draw_paddle(win, state.player2_x, state.player2_y, 2);
        draw_ball(win, state.ball_x, state.ball_y);
        wrefresh(win);
    }

    close(sock);
    endwin();
    printf("Rozlaczono z serwerem.\n");
    return 0;
}
