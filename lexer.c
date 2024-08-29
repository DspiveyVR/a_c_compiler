#include <stdio.h>
#include <string.h>

typedef struct Token {
    /* data */
} Token;

//TODO: static arrays for now, will implement dynamic later.
void lex(char* file_text) {
// void lex(Token** tokens, char* file_text) {
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
                memcpy(words[words_index], word, 50);
                memset(word, 0, 50);
                word_index = 0;
                words_index++;
                continue;
            default:
                word[word_index] = file_text[file_index];
                word_index++;
                break;
        }
    }
    end_loop:

    for (int i = 0; i < 255; i++) {
        printf("%d %s ",i , words[i]);        
    }
}