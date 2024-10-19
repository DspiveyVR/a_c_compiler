#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dynamic_string.h"

typedef enum TokenType {
    NONE,
    KEYWORD,
    IDENTIFIER,
    EQUALS,
    EQUALS_EQUALS,
    CONSTANT,
    STRING_LITERAL,
    LEFT_PARENTHESIS,
    RIGHT_PARENTHESIS,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    SEMICOLON,
    COMMA,
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

typedef struct StringList {
    String** list;
    int length;
} StringList;


void set_tklist_string(TokenList* array, int index, char* string);
void set_tklist_type(TokenList* array, int index, TokenType type);
void initialize_string_list(StringList* array, int length);
void destroy_string_list(StringList* array);
void set_string_in_list(StringList* array, int index, char* string, int bytes);

void lex(TokenList* tokens, String* file_text) {
    // A "word" is a string that corresponds to a token.
    // char words[256][50] = {0}; 
    StringList words;
    initialize_string_list(&words, 100);
    int words_index = 0;

    // char word[50] = {0};
    String word;
    initialize_string(&word, 10);
    int word_index = 0;
    for (int file_index = 0; file_index < file_text->length - 1; file_index++) {
        switch (file_text->string[file_index]) {
            case 0:
                set_string_in_list(&words, words_index, word.string, word.length);
                goto end_loop;
            case ' ':
            case '\t':
            case '\n':
                // Ensures that no empty words are added.
                if (strncmp(word.string, "", word.length) != 0) {
                    set_string_in_list(&words, words_index, word.string, word.length);
                    memset(word.string, 0, word.length);
                    word_index = 0;
                    words_index++;
                }
                break;
            case '=':
                if (file_text->string[file_index + 1] == '=') {
                    set_char(&word, '=', 0);
                    set_char(&word, '=', 1);
                    set_string_in_list(&words, words_index, word.string, word.length);
                    memset(word.string, 0, word.length);
                    words_index++;
                    file_index++;
                    break;
                }
            case '(':
            case ')':
            case '{':
            case '}':
            case ';':
            case '+':
            case '*':
            case ',':
                if (strncmp(word.string, "", word.length) != 0) {
                    set_string_in_list(&words, words_index, word.string, word.length);
                    memset(word.string, 0, word.length);
                    word_index = 0;
                    words_index++;
                }
                set_char(&word, file_text->string[file_index], 0);
                set_string_in_list(&words, words_index, word.string, word.length);
                memset(word.string, 0, word.length);
                words_index++;
                break;
            default:
                set_char(&word, file_text->string[file_index], word_index);
                word_index++;
                break;
        }
    }
    end_loop:
    free(word.string);

    for (int i = 0; i < words_index; i++) {
        bool isnumber = true;
        for (int j = 0; j < words.list[i]->length; j++) {
            if (!isdigit(words.list[i]->string[j]) 
                && words.list[i]->string[j] != 0) {
                isnumber = false;
                break;
            }
        }
        if (strncmp(words.list[i]->string, "int", 3) == 0) {
            set_tklist_type(tokens, i, KEYWORD);
            set_tklist_string(tokens, i, "int");
        } else if (isnumber) {
            set_tklist_type(tokens, i, CONSTANT);
            set_tklist_string(tokens, i, words.list[i]->string);
        } else if (strncmp(words.list[i]->string, "return", 6) == 0) {
            set_tklist_type(tokens, i, KEYWORD);
            set_tklist_string(tokens, i, "return");
        } else if (strncmp(words.list[i]->string, "if", 6) == 0) {
            set_tklist_type(tokens, i, KEYWORD);
            set_tklist_string(tokens, i, "if");
        } else if (strncmp(words.list[i]->string, "==", 2) == 0) {
            set_tklist_type(tokens, i, EQUALS_EQUALS);
        } else if (strncmp(words.list[i]->string, "=", 1) == 0) {
            set_tklist_type(tokens, i, EQUALS);
        } else if (strncmp(words.list[i]->string, "(", 1) == 0) {
            set_tklist_type(tokens, i, LEFT_PARENTHESIS);
        } else if (strncmp(words.list[i]->string, ")", 1) == 0) {
            set_tklist_type(tokens, i, RIGHT_PARENTHESIS);
        } else if (strncmp(words.list[i]->string, "{", 1) == 0) {
            set_tklist_type(tokens, i, LEFT_BRACKET);
        } else if (strncmp(words.list[i]->string, "}", 1) == 0) {
            set_tklist_type(tokens, i, RIGHT_BRACKET);
        } else if (strncmp(words.list[i]->string, ";", 1) == 0) {
            set_tklist_type(tokens, i, SEMICOLON);
        } else if (strncmp(words.list[i]->string, ",", 1) == 0) {
            set_tklist_type(tokens, i, COMMA);
        } else if (strncmp(words.list[i]->string, "+", 1) == 0) {
            set_tklist_type(tokens, i, PLUS);
        } else if (strncmp(words.list[i]->string, "*", 1) == 0) {
            set_tklist_type(tokens, i, STAR);
        } else if (isalpha(words.list[i]->string[0]) || 
                   words.list[i]->string[0] == '_') {
            set_tklist_type(tokens, i, IDENTIFIER);
            set_tklist_string(tokens, i, words.list[i]->string);                           
        } else {
            set_tklist_type(tokens, i, INVALID);
            set_tklist_string(tokens, i, words.list[i]->string);
        }
    }
    destroy_string_list(&words);

    if (words_index < tokens->length) {
        for (int i = words_index; i < tokens->length; i++) {
            set_tklist_type(tokens, i, NONE);
        }
    }
}

void initialize_token_list(TokenList* array, int length) {
    array->tokens = malloc(sizeof(Token*) * length);
    array->length = length;

    for (int i = 0; i < length; i++) {
        array->tokens[i] = malloc(sizeof(Token));
        array->tokens[i]->value = calloc(50, 1);
    }
}

void destroy_token_list(TokenList* array) {
    for (int i = 0; i < array->length; i++) {
        free(array->tokens[i]->value);
        free(array->tokens[i]);
    }
    free(array->tokens);
}

void set_tklist_string(TokenList* array, int index, char* string) {
    if (index >= array->length) {
        array->tokens = realloc((void*)(array->tokens), (array->length + 5) * sizeof(Token*));
        array->length += 5;
        for (int i = array->length - 5; i < array->length; i++) {
            array->tokens[i] = malloc(sizeof(Token));
            array->tokens[i]->value = calloc(50, 1);
        }
    }
    strncpy(array->tokens[index]->value, string, 50);
}

void set_tklist_type(TokenList* array, int index, TokenType type) {
    if (index >= array->length) {
        array->tokens = realloc((void*)(array->tokens), (array->length + 5) * sizeof(Token*));
        array->length += 5;
        for (int i = array->length - 5; i < array->length; i++) {
            array->tokens[i] = malloc(sizeof(Token));
            array->tokens[i]->value = calloc(50, 1); 
        }
    }
    array->tokens[index]->type = type;
}

void initialize_string_list(StringList* array, int length) {
    array->list = malloc(sizeof(String*) * length);
    array->length = length;   

    for (int i = 0; i < length; i++) {
        array->list[i] = malloc(sizeof(String));
        initialize_string(array->list[i], 10);
    }
}

void destroy_string_list(StringList* array) {
    for (int i = 0; i < array->length; i++) {
        free(array->list[i]->string);
        free(array->list[i]);
    }
    free(array->list);
}

void set_string_in_list(StringList* array, int index, char* string, int bytes) {
    if (index >= array->length) {
        array->list = realloc((void*)(array->list), (array->length + 5) * sizeof(String*));
        array->length += 5;

        for (int i = array->length - 5; i < array->length; i++) {
            array->list[i] = malloc(sizeof(String*));
            initialize_string(array->list[i], 10);
        }
    }

    set_string(array->list[index], string, bytes);
}
