#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "dynamic_string.h"
#include "parser.h"

void enum_to_str(char* buf, TokenType type);
void print_exp(Node* node);
int eval_exp(Node* node);

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
        char type[20] = "";
        enum_to_str(type, tokens.tokens[i]->type);

        printf("%s %s\n", type, tokens.tokens[i]->value);
    }

    /*destroy_token_list(&tokens);*/

    /*TokenList tokens;*/
    /*initialize_token_list(&tokens, 5);*/
    /*set_tklist_type(&tokens, 0, CONSTANT);*/
    /*set_tklist_string(&tokens, 0, "2");*/
    /*set_tklist_type(&tokens, 1, PLUS);*/
    /*set_tklist_type(&tokens, 2, CONSTANT);*/
    /*set_tklist_string(&tokens, 2, "3");*/
    /*set_tklist_type(&tokens, 3, STAR);*/
    /*set_tklist_type(&tokens, 4, CONSTANT);*/
    /*set_tklist_string(&tokens, 4, "4");*/
    Node* expression = parse_expression(&tokens);
    destroy_token_list(&tokens);
    print_exp(expression);
    int result = eval_exp(expression);
    destroy_node(expression);
    printf("= %d", result);
    printf("\n");

    return 0;
}

void print_exp(Node* node) {
    /*printf("(");*/
    if (node->lhs != NULL) {
        print_exp(node->lhs);
    }
    switch (node->type) {
        case INTEGER:
            printf("%u", (uint32_t)node->value);
            break;
        case ADDITION:
            printf("+");
            break;
        case MULTIPLICATION:
            printf("*");
            break;
        case OPEN_PARENTHESIS:
            printf("(");
            break;
        case CLOSE_PARENTHESIS:
            printf(")");
            break;
        case NONETYPE:
            printf("NONETYPE");
            break;
    }
    if (node->rhs != NULL) {
        print_exp(node->rhs);
    }
    /*printf(")");*/
}

int eval_exp(Node* node) {
    switch (node->type) {
        case INTEGER:
            return node->value;
            break;
        case ADDITION:
            return eval_exp(node->lhs) + eval_exp(node->rhs);
            break;
        case MULTIPLICATION:
            return eval_exp(node->lhs) * eval_exp(node->rhs);
            break;
        case OPEN_PARENTHESIS:
            return eval_exp(node->rhs);
            break;
        case CLOSE_PARENTHESIS:
            break;
        case NONETYPE:
            return eval_exp(node->rhs);
            break;
    }

    return -1;
}

void enum_to_str(char* buf, TokenType type) {
    switch (type) {
        case KEYWORD:
            strncpy(buf, "KEYWORD", 8);
            break;
        case IDENTIFIER:
            strncpy(buf, "IDENTIFIER", 11);
            break;
        case CONSTANT:
            strncpy(buf, "CONSTANT", 9);
            break;
        case PLUS:
            strncpy(buf, "PLUS", 5);
            break;
        case STAR:
            strncpy(buf, "STAR", 5);
            break;
        case STRING_LITERAL:
            strncpy(buf, "STRING_LITERAL", 15);
            break;
        case LEFT_PARENTHESIS:
            strncpy(buf, "LEFT_PARENTHESIS", 17); 
            break;
        case RIGHT_PARENTHESIS:
            strncpy(buf, "RIGHT_PARENTHESIS", 18);
            break;
        case LEFT_BRACKET:
            strncpy(buf, "LEFT_BRACKET", 13);
            break;
        case RIGHT_BRACKET:
            strncpy(buf, "RIGHT_BRACKET", 14);
            break;
        case SEMICOLON:
            strncpy(buf, "SEMICOLON", 10);
            break;
        case INVALID:
            strncpy(buf, "INVALID", 8);
            break;
        case NONE:
            strncpy(buf, "NONE", 5);
            break;
    }
}
