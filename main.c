#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <curses.h>
#include <form.h>

#include "helper.h"

void setsig();

static const char passwd[] = "123456";
static int bail_request = 0;
static int row, col;

#define ASZ(z) (sizeof(z) / sizeof(z[0]))
#define KEY_DEL 127

struct mainopt {
    char name[15];
    void (*trigger)();
};

void bail() {
    bail_request = 1;
}

void window_title(WINDOW *win, int starty, int startx, int width, char *string, chtype color) {
    int x, y;
    float temp;
    if (!win)
        win = stdscr;

    /* Window title */
    wattron(win, color);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 2, "%s", string);
    wattroff(win, A_BOLD);
    wattroff(win, color);
}

void new_entry() {
    FIELD *field[4];
    FORM  *my_form;
    WINDOW *my_form_win;
    int ch, rows, cols;

    curs_set(1);

    /* Initialize the fields */
    field[0] = new_field(1, 15, 4, 10, 0, 0);
    field[1] = new_field(1, 15, 6, 10, 0, 0);
    field[2] = new_field(1, 1, 8, 20, 0, 0);
    field[3] = NULL;

    /* Set field options */
    set_field_back(field[0], A_UNDERLINE);
    field_opts_off(field[0], O_AUTOSKIP); 
                                          
    set_field_back(field[1], A_UNDERLINE); 
    field_opts_off(field[1], O_AUTOSKIP);

    set_field_back(field[2], A_REVERSE);
    field_opts_off(field[2], O_AUTOSKIP);
    field_opts_off(field[2], O_EDIT);
    field_opts_off(field[2], O_STATIC);

    /* Create the form and post it */
    my_form = new_form(field);
    scale_form(my_form, &rows, &cols);

    /* Create the window to be associated with the form */
    my_form_win = newwin(13, 30, (row/2)-6, (col/2)-15);
    keypad(my_form_win, TRUE);

    /* Set main window and sub window */
    set_form_win(my_form, my_form_win);
    set_form_sub(my_form, derwin(my_form_win, rows, cols, 2, 2));

    /* Print a border around the main window and print a title */
    box(my_form_win, 0, 0);
    post_form(my_form);
    window_title(my_form_win, 1, 0, cols + 4, "NEW ENTRY", COLOR_PAIR(1));
    wrefresh(my_form_win);
    refresh();

    /* Window description */
    mvwprintw(my_form_win, 3, 2, "ADD NEW ENTRY");

    /* Field labels */
    mvwprintw(my_form_win, 6, 2, "NAME:");
    mvwprintw(my_form_win, 8, 2, "OPTION:");

    /* Sumbit button */
    wattron(my_form_win, A_REVERSE);
    mvwprintw(my_form_win, 10, 20, " DONE ");
    wattron(my_form_win, A_REVERSE);

    /* Set cursor at first field */
    form_driver(my_form, REQ_FIRST_FIELD);

    /* Loop through to get user requests */
    while ((ch = wgetch(my_form_win)) != 10) {
        switch (ch) {
            case KEY_DOWN:
            case 9: /* Tab */

                /* Go to next field */
                form_driver(my_form, REQ_NEXT_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;

            case KEY_UP:

                /* Go to previous field */
                form_driver(my_form, REQ_PREV_FIELD);
                form_driver(my_form, REQ_END_LINE);
                break;

            case KEY_BACKSPACE:
            case KEY_DEL:

                /* Erase char */
                form_driver(my_form, REQ_DEL_PREV);
                break;

            default:

                /* Print char */
                form_driver(my_form, ch);
                break;
        }
    }

    /* Un post form and free the memory */
    unpost_form(my_form);
    free_form(my_form);
    free_field(field[0]);
    free_field(field[1]);
    free_field(field[2]);

    curs_set(0);
    delwin(my_form_win);
    clear();
}

struct mainopt choices[] = {
        {"NEW ENTRY", new_entry},
        {"  SEARCH", NULL},
        {"  LIST", NULL},
        {"  CATEGORIES", NULL},
        {"  TAGS", NULL},
        {"", NULL},
        {"FLUSH", NULL},
        {"HELP", NULL},
        {"EXIT", bail}
};

void print_menu(WINDOW *menu_win, int highlight) {
        int x, y, i;
        x = 2;
        y = 2;
        box(menu_win, 0, 0);
        for (i = 0; i < ASZ(choices); ++i) {
                if (highlight == i + 1) {
                        wattron(menu_win, A_REVERSE);
                        mvwprintw(menu_win, y, x, "%s", choices[i].name);
                        wattroff(menu_win, A_REVERSE);
                } else {
                        mvwprintw(menu_win, y, x, "%s", choices[i].name);
                }
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

redo:
    print_menu(menu, highlight);
    while (1) {
            int c = wgetch(menu);
            switch (c) {
                    case KEY_UP:
                            if (highlight == 1)
                                    highlight = ASZ(choices);
                            else
                                    --highlight;
                            break;
                    case KEY_DOWN:
                            if (highlight == ASZ(choices))
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
            if (choice != 0)
                break;
    }

    /* Call the handler */
    if (choices[choice - 1].trigger) {
        choices[choice - 1].trigger();
        highlight = 1;
        choice = 0;
    }
    refresh();

    if (bail_request) {
        curs_set(1);
        delwin(menu);
        return;
    }

    goto redo;
}

void main_content() {
    int highlight = 1;
    int choice = 0;
    WINDOW *content = newwin(row - 3, col - 20, 2, 20);

    box(content, 0, 0);
    wrefresh(content);
    delwin(content);
}

void handle_winch(int sig){
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    col = w.ws_col;
    row = w.ws_row;

    wresize(stdscr, row, col);
    clear();

    refresh();
}

void mainw() {
    clear();
    mvprintw(1, (col - 7) / 2, "--<[ TISR ]>--");
    mvprintw(row - 1, 1, "DBOPEN | ");
    refresh();

    // setsig();

    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);

    main_content();
    main_menu();
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
        if (ch == KEY_BACKSPACE || ch == KEY_DEL) {
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
        } else {
            mvprintw(12, 2, "PASSWORD: %d\n", ch);
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
    // if (loginw()) {
        mainw();

        // clear();
        // mvaddstr(1, 2, "CLOSING DATABASE");
        // refresh();
        // sleep(1);
    // }

    /*  Clean up */
    clrtoeol();
    endwin();

    return 0;
}

/*  Signal handler  */
void handler(int signum) {
    switch (signum) {
        case SIGALRM:

            /* Received from the timer  */
            break;

        case SIGTERM:
        case SIGINT:

            /*  Clean up nicely  */
            // delwin(mainwin);
            // curs_set(1);
            // endwin();
            // refresh();
            // FreeWorm();
            // exit(EXIT_SUCCESS);
            break;
    }
}

/*  Sets up signal handlers we need  */
void setsig() {
    struct sigaction sa;

    /* Fill in sigaction struct  */
    sa.sa_handler = handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    /* Set signal handlers  */
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGALRM, &sa, NULL);

    /* Ignore SIGTSTP  */
    sa.sa_handler = SIG_IGN;
    sigaction(SIGTSTP, &sa, NULL);
}
