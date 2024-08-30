#include <stdlib.h>
#include "lexer.h"

typedef struct TokenList {
    Token** tokens;
    int length;
} TokenList;

void initialize_token_list(TokenList* array, int length) {
    array->tokens = malloc(8 /* pointer size */ * length);
    array->length = length;

    for (int i = 0; i < length; i++) {
        array->tokens[i] = (Token*)malloc(sizeof(Token));
        array->tokens[i]->value = (char*)malloc(50);
    }
}

void destroy_token_list(TokenList* array) {
    for (int i = 0; i < array->length; i++) {
        free(array->tokens[i]->value);
        free(array->tokens[i]);
    }
    free(array);
}

void set_tklist_element(TokenList* array, int index, Token* item) {
    if (index >= array->length) {
        array->tokens = realloc((void*)(array->tokens), array->length + 5);
    }
    array->tokens[index] = item;
}
