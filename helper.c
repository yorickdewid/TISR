#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <ctype.h>

/*  Quit on error  */
// void Error_Quit(char * msg) {
//     extern WINDOW * mainwin;
//     extern int oldcur;

//     /*  Clean up nicely  */
//     delwin(mainwin);
//     curs_set(oldcur);
//     endwin();
//     refresh();

//     /*  Output error message and exit  */
//     perror(msg);
//     exit(EXIT_FAILURE);
// }

/*  Quit successfully  */
// void Quit(int reason) {
//     extern WINDOW * mainwin;
//     extern int oldcur;
//     extern int score;

//     /*  Clean up nicely  */
//     delwin(mainwin);
//     curs_set(oldcur);
//     endwin();
//     refresh();

//     /*  Output farewell message  */
//     switch (reason) {
//         case HITWALL:
//             printf("\nYou hit a wall!\n");
//             printf("Your score is %d\n", score);
//             break;

//         case HITSELF:
//             printf("\nYou ran into yourself!\n");
//             printf("Your score is %d\n", score);
//             break;

//         default:
//             printf("\nGoodbye!\n");
//             break;
//     }

//     exit(EXIT_SUCCESS);
// }

/*  Returns the x-y size of the terminal  */
void get_term_size(int *rows, int *cols) {
    struct winsize ws;

    /* Get terminal size */
    if (ioctl(0, TIOCGWINSZ, &ws) < 0) {
        // perror("couldn't get window size");
        exit(EXIT_FAILURE);
    }

    /*  Update globals  */
    *rows = ws.ws_row;
    *cols = ws.ws_col;
}

char *trim_white_space(char *str) {
    char *end;

    /* Trim leading space */
    while (isspace(*str))
        str++;

    /* All spaces? */
    if (*str == 0)
        return str;

    /* Trim trailing space */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;

    /* Write new null terminator */
    *(end + 1) = 0;

    return str;
}

/*  Signal handler  */
// void handler(int signum) {
//     extern WINDOW * mainwin;
//     extern int oldcur;


//     /*  Switch on signal number  */

//     switch ( signum ) {

//     case SIGALRM:

//         /*  Received from the timer  */

//         MoveWorm();
//         return;

//     case SIGTERM:
//     case SIGINT:

//         /*  Clean up nicely  */

//         delwin(mainwin);
//         curs_set(oldcur);
//         endwin();
//         refresh();
//         FreeWorm();
//         exit(EXIT_SUCCESS);

//     }
// }
