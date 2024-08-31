#include <string.h>
#include <stdlib.h>

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
    char* value;
} Token;

typedef struct TokenList {
    Token** tokens;
    int length;
} TokenList;


void set_tklist_string(TokenList* array, int index, char* string);
void set_tklist_type(TokenList* array, int index, TokenType type);

void lex(TokenList* tokens, char* file_text) {
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

    for (int i = 0; i < words_index; i++) {
        if (strncmp(words[i], "int", 3) == 0) {
            set_tklist_type(tokens, i, KEYWORD);
            set_tklist_string(tokens, i, "int");
        } else if (strncmp(words[i], "return", 6) == 0) {
            set_tklist_type(tokens, i, KEYWORD);
            set_tklist_string(tokens, i, "return");
        } else if (strncmp(words[i], "main", 5) == 0) {
            set_tklist_type(tokens, i, IDENTIFIER);
            set_tklist_string(tokens, i, "main");
        } else if (strncmp(words[i], "(", 1) == 0) {
            set_tklist_type(tokens, i, PUNCTUATOR);
            set_tklist_string(tokens, i, "(");
        } else if (strncmp(words[i], ")", 1) == 0) {
            set_tklist_type(tokens, i, PUNCTUATOR);
            set_tklist_string(tokens, i, ")");
        } else if (strncmp(words[i], "{", 1) == 0) {
            set_tklist_type(tokens, i, PUNCTUATOR);
            set_tklist_string(tokens, i, "{");
        } else if (strncmp(words[i], "}", 1) == 0) {
            set_tklist_type(tokens, i, PUNCTUATOR);
            set_tklist_string(tokens, i, "}");
        } else if (strncmp(words[i], ";", 1) == 0) {
            set_tklist_type(tokens, i, PUNCTUATOR);
            set_tklist_string(tokens, i, ";");
        } else {
            set_tklist_type(tokens, i, INVALID);
            set_tklist_string(tokens, i, words[i]);
        }
    }
}

void initialize_token_list(TokenList* array, int length) {
    array->tokens = malloc(sizeof(Token*) * length);
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
}

void set_tklist_string(TokenList* array, int index, char* string) {
    if (index >= array->length) {
        array->tokens = realloc((void*)(array->tokens), (array->length + 5) * sizeof(Token*));
        array->length += 5;
        for (int i = array->length - 5; i < array->length; i++) {
            array->tokens[i] = (Token*)malloc(sizeof(Token));
            array->tokens[i]->value = (char*)malloc(50);           
        }
    }
    strncpy(array->tokens[index]->value, string, 50);
}

void set_tklist_type(TokenList* array, int index, TokenType type) {
    if (index >= array->length) {
        array->tokens = realloc((void*)(array->tokens), (array->length + 5) * sizeof(Token*));
        array->length += 5;
        for (int i = array->length - 5; i < array->length; i++) {
            array->tokens[i] = (Token*)malloc(sizeof(Token));
            array->tokens[i]->value = (char*)malloc(50);           
        }
    }
    array->tokens[index]->type = type;
}
