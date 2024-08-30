#include <string.h>

typedef enum TokenType {
    KEYWORD,
    IDENTIFIER,
    CONSTANT,
    STRING_LITERAL,
    PUNCTUATOR, 
    INVALID,
} TokenType;

typedef struct Token {
    TokenType type;
    char value[50];
} Token;

//TODO: static arrays for now, will implement dynamic later.
void lex(Token* tokens, char* file_text) {
    // A "word" is a string that corresponds to a token.
    char words[256][50] = {0}; 
    int words_index = 0;

    char word[50] = {0};
    int word_index = 0;
    for (int file_index = 0; file_index < 0xffff - 1; file_index++) {
        switch (file_text[file_index]) {
            case 0:
                memcpy(words[words_index], word, 50);
                goto end_loop;
            case ' ':
            case '\t':
            case '\n':
                // Ensures that no empty words are added.
                if (strncmp(word, "", 50) == 0) {
                    continue;
                }
                memcpy(words[words_index], word, 50);
                memset(word, 0, 50);
                word_index = 0;
                words_index++;
                continue;
            case '(':
            case ')':
            case '{':
            case '}':
            case ';':
                memcpy(words[words_index], word, 50);
                memset(word, 0, 50);
                word_index = 0;
                words_index++;
            default:
                word[word_index] = file_text[file_index];
                word_index++;
                break;
        }
    }
    end_loop:

    for (int i = 0; i < 255; i++) {
        if (strncmp(words[i], "int", 50) == 0) {
            tokens[i].type = KEYWORD;
            strcpy(tokens[i].value, "int");
        }
    }
}
