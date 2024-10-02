#include <stdio.h>    
#include <unistd.h>
//theyaremanycolors

void clear(void) {
    printf("\033[0m");
}

void bg_red(void) {
    printf("\033[0;41m");
}

void bg_magenta(void) {
    printf("\033[0;45m");
}

void fg_green(void) {
    printf("\033[0;32m");
}

void fg_bright_red(void) {
    printf("\033[0;91m");
}

void fg_cyan(void) {
    printf("\033[0;36m");
}

void bold(void) {
    printf("\033[1m");
}

void clear_bold(void) {
    printf("\033[22m");   
}

void dim(void) {
    printf("\033[2m");
}
