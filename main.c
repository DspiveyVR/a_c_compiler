#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "dynamic_string.h"

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

    String file_text;
    initialize_string(&file_text, 20);
    int i = 0;
    while (!feof(infile)) {
        set_char(&file_text, fgetc(infile), i);
        i++;
    } 
    set_char(&file_text, 0, i);
    fclose(infile);

    // TODO: perhaps make a none type for the token enum
    TokenList tokens;
    initialize_token_list(&tokens, 5);

    lex(&tokens, &file_text);
    free(file_text.string);

    for (int i = 0; i < tokens.length; i++) {
        // strncpy(tokens[i].value, "int", 3);
        char type[14] = "";
        enum_to_str(type, tokens.tokens[i]->type);

        printf("%s %s\n", type, tokens.tokens[i]->value);
    }

    destroy_token_list(&tokens);

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
        case NONE:
            strncpy(buf, "NONE", 4);
            break;
    }
}
