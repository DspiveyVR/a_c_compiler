#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"

void enum_to_str(char* buf, TokenType type);

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("invalid arguments\n");
        exit(1);
    }
    FILE* infile;
    infile = fopen(argv[1], "r");
    if (infile == NULL) {
        printf("error opening file\n");
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
    // Token** tokens = {0};
    // tokens = (Token**)malloc(sizeof(Token**) * 256);
    // for (int i = 0; i < 256; i++) {
    //     tokens[i] = (Token*)malloc(sizeof(Token));
    //     tokens[i]->value = (char*)malloc(50);
    // }
    lex(tokens, file_text);

    for (int i = 0; i < 10; i++) {
        // strncpy(tokens[i].value, "int", 3);
        char type[14] = "";
        enum_to_str(type, tokens[i]->type);

        printf("%s %s\n", type, tokens[i]->value);
    }

    for (int i = 0; i < 256; i++) {
        free(tokens[i]->value);
        free(tokens[i]);
    }
    free(tokens);

    return 0;
}

void enum_to_str(char* buf, TokenType type) {
    switch (type) {
        case KEYWORD:
            strncpy(buf, "KEYWORD", 7);
            break;
        case IDENTIFIER:
            strncpy(buf, "IDENTIFIER", 10);
            break;
        case CONSTANT:
            strncpy(buf, "CONSTANT", 8);
            break;
        case STRING_LITERAL:
            strncpy(buf, "STRING_LITERAL", 14);
            break;
        case PUNCTUATOR:
            strncpy(buf, "PUNCTUATOR", 10);
            break;
        case INVALID:
            strncpy(buf, "INVALID", 7);
            break;
    }
}
