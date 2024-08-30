#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("invalid arguments\n");
        exit(1);
    }
    FILE* infile;
    infile = fopen(argv[1], "r");
    if (infile == NULL) {
        printf("error opening file");
        exit(1);
    }
    // TODO: make dynamic
    char file_text[0xffff] = {0};
    for (int i = 0; i < 0xffff; i++) {
        if (!feof(infile)) {
            file_text[i] = fgetc(infile);
        } else {
            file_text[i - 1] = 0;
            break;
        }
    }
    fclose(infile);
    // printf("%s\n", file_text);
    Token tokens[256] = {0};
    lex(tokens, file_text);


    return 0;
}
