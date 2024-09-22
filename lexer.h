#ifndef LEXER_H
#define LEXER_H
#include "dynamic_string.h"

typedef enum TokenType {
    NONE,
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    STRING_LITERAL,
    PUNCTUATOR, 
    PLUS,
    STAR,
    INVALID,
} TokenType;

typedef struct Token {
    TokenType type;
    char* value;
} Token;

typedef struct TokenList {
    Token** tokens;
    int length;
} TokenList;

void lex(TokenList* tokens, String* file_text);
void initialize_token_list(TokenList* array, int length);
void destroy_token_list(TokenList* array);
void set_tklist_string(TokenList* array, int index, char* string);
void set_tklist_type(TokenList* array, int index, TokenType type);

#endif
