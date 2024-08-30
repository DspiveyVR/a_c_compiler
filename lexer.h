#ifndef LEXER_H
#define LEXER_H

typedef enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    STRING_LITERAL,
    PUNCTUATOR, 
} TokenType;

typedef struct Token {
    TokenType type;
    char* value;
} Token;


void lex(Token* tokens, char* file_text);

#endif
