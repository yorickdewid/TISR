#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <curses.h>

char *intprtkey(int ch);

static const char passwd[] = "123456";
int row, col;

char *choices[] = { 
        "NEW ENTRY",
        "|-SEARCH",
        "|-LIST",
        "|-CATEGORIES",
        "|-TAGS",
        "FLUSH",
        "EXIT",
};
int n_choices = sizeof(choices) / sizeof(choices[0]);

void print_menu(WINDOW *menu_win, int highlight) {
        int x, y, i;
        x = 2;
        y = 2;
        box(menu_win, 0, 0);
        for(i = 0; i < n_choices; ++i)
        {       if(highlight == i + 1) /* High light the present choice */
                {       wattron(menu_win, A_REVERSE); 
                        mvwprintw(menu_win, y, x, "%s", choices[i]);
                        wattroff(menu_win, A_REVERSE);
                }
                else
                        mvwprintw(menu_win, y, x, "%s", choices[i]);
                ++y;
        }
        wrefresh(menu_win);
}

void main_menu() {
    int highlight = 1;
    int choice = 0;
    WINDOW *menu = newwin(row - 3, 20, 2, 0);

    curs_set(0);

    keypad(menu, TRUE);

retry:
    print_menu(menu, highlight);
    while (1) {
            int c = wgetch(menu);
            switch (c) {
                    case KEY_UP:
                            if(highlight == 1)
                                    highlight = n_choices;
                            else
                                    --highlight;
                            break;
                    case KEY_DOWN:
                            if(highlight == n_choices)
                                    highlight = 1;
                            else 
                                    ++highlight;
                            break;
                    case 10:
                            choice = highlight;
                            break;
                    default:
                            break;
            }
            print_menu(menu, highlight);
            if (choice != 0) /* User did a choice come out of the infinite loop */
                break;
    }

    if (choice == 7)
        curs_set(1);
    else
        goto retry;
}

void mainw() {
    WINDOW *win = newwin(row - 3, col, 2, 0);

    clear();
    mvprintw(1, (col - 7) / 2, "--<[ TISR ]>--");
    mvprintw(row - 1, 1, "DBOPEN | ");
    refresh();

    // box(win, 0, 0);
    wrefresh(win);

    main_menu();

    /* Done */
    delwin(win);
}

int loginw() {
    int ch, chsz = 0, succes = 0;
    char passbuf[64];
    WINDOW *win = newwin(col, row, 0, 0);

    keypad(win, TRUE);
    refresh();

    /*  Print a prompt and refresh() the screen  */
    mvaddstr(1, 2, "ACCESS TISR CONSOLE");
    mvprintw(3, 2, "PRESS <ESC> TO QUIT");
    mvprintw(5, 2, "PASSWORD: ");

    /*  Loop until user presses 'q'  */
    while ((ch = getch()) != 0x1b) {
        int j = 0;
        if (ch == KEY_BACKSPACE) {
            if (chsz > 0) {
                chsz--;
                deleteln();
            }
        } else if (ch == 10) {
            deleteln();
            if (chsz < 5) {
                chsz = 0;
            } else {
                if (!strncmp(passbuf, passwd, chsz)) {
                    succes = 1;
                    break;
                }

                // sleep(5);
                chsz = 0;
            }
        } else if (isprint(ch)) {
            passbuf[chsz++] = ch;
        }

        mvprintw(5, 2, "PASSWORD: ");
        for (j=chsz; j--; addch('*'));
        refresh();
    }

    /* Done */
    delwin(win);
    return succes;
}

int main(int argc, char *argv[]) {
    initscr();
    getmaxyx(stdscr, row, col);
    clear();
    noecho();

    /* Login window */
    if (loginw()) {
        mainw();

        clear();
        mvaddstr(1, 2, "CLOSING DATABASE");
        refresh();
        sleep(1);
    }

    /*  Clean up */
    clrtoeol();
    endwin();

    return 0;
}
