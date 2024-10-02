#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "dynamic_string.h"
#include "parser.h"
#include "code_generation.h"
#include "colors.h"

void enum_to_str(char* buf, TokenType type);
/*void print_exp(Node* node);*/
/*int eval_exp(Node* node);*/
void print_ast(Node* node);

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

    TokenList tokens;
    initialize_token_list(&tokens, 5);

    lex(&tokens, &file_text);
    free(file_text.string);

    bg_red();
    bold();
    printf("TOKENS");
    clear();
    printf("\n");
    for (int i = 0; i < tokens.length; i++) {
        // strncpy(tokens[i].value, "int", 3);
        char type[20] = "";
        enum_to_str(type, tokens.tokens[i]->type);

        printf("  %s %s\n", type, tokens.tokens[i]->value);
    }
    /*Node* expression = parse_expression(&tokens);*/
    NodeList statements;
    initialize_node_list(&statements, 3);
    int token_index = 0;
    for (int i = 0; token_index < tokens.length; i++) {
        if (i >= statements.capacity) {
            increase_node_list_capacity(&statements, statements.capacity + 5);
        }
        statements.nodes[i] = parse_statement(&tokens, &token_index);
    }
    destroy_token_list(&tokens);
    bg_magenta();
    bold();
    printf("AST");
    clear();
    for (int i = 0; statements.nodes[i] != NULL && i < statements.capacity; i++) {
        printf("\n  ");
        print_ast(statements.nodes[i]);
    }
    printf("\n");
    for (int i = 0; statements.nodes[i] != NULL && i < statements.capacity; i++) {
        destroy_node(statements.nodes[i]);
    }
    destroy_node_list(&statements);
    /*String asm_text;*/
    /*initialize_string(&asm_text, 10);*/
    /*generate_assembly(&asm_text ,expression);*/
    /*printf("%s", asm_text.string);*/
    /*FILE* asm_file;*/
    /*asm_file = fopen("asm.s", "w");*/
    /*if (asm_file == NULL) {*/
    /*    printf("error opening file\n");*/
    /*    exit(1);*/
    /*}*/
    /*fprintf(asm_file, "%s", asm_text.string);*/
    /*fclose(asm_file);*/
    /*free(asm_text.string);*/
    /*destroy_node(expression);*/


    return 0;
}

void print_ast(Node* node) {
    if (node == NULL) {
        return;
    }
    switch (node->type) {
        case INTEGER_E:
            clear();
            printf("%u", (uint32_t)node->value);
            break;
        case VARIABLE_E:
            clear();
            fg_green();
            printf("%s", node->name);
            break;
        case ADDITION_E:
            clear();
            fg_cyan();
            bold();
            printf("ADD");
            clear_bold();
            dim();
            printf("(");
            print_ast(node->lhs);
            clear();
            fg_cyan();
            dim();
            printf(",");
            print_ast(node->rhs);
            clear();
            fg_cyan();
            dim();
            printf(")");
            break;
        case MULTIPLICATION_E:
            clear();
            fg_bright_red();
            bold();
            printf("MUL");
            clear_bold();
            dim();
            printf("(");
            print_ast(node->lhs);
            clear();
            fg_bright_red();
            dim();
            printf(",");
            print_ast(node->rhs);
            clear();
            fg_bright_red();
            dim();
            printf(")");
            break;
        case OPEN_PARENTHESIS_E:
            print_ast(node->rhs);
            break;
        case CLOSE_PARENTHESIS_E:
            break;
        case VARIABLE_DECLARATION_S:
            clear();           
            // INT is only type so it's hardcoded for now.
            printf("VAR_DECLARATION(%s, %s)", "INT", node->name);
            break;
        case ASSIGNMENT_E:
            clear();
            printf("ASSIGNMENT(");
            print_ast(node->lhs);
            clear();
            printf(",");
            print_ast(node->rhs);
            clear();
            printf(")");
            break;
        case RETURN_S:
            clear();
            printf("RETURN(");
            print_ast(node->rhs);
            clear();
            printf(")");
            break;
        case NONETYPE:
            clear(); 
            printf("NONETYPE");
            break;
    }

    return;
}

/*
void print_exp(Node* node) {
    if (node->lhs != NULL) {
        print_exp(node->lhs);
    }
    switch (node->type) {
        case INTEGER_E:
            printf("%u", (uint32_t)node->value);
            break;
        case ADDITION_E:
            printf("+");
            break;
        case MULTIPLICATION_E:
            printf("*");
            break;
        case OPEN_PARENTHESIS_E:
            printf("(");
            break;
        case CLOSE_PARENTHESIS_E:
            printf(")");
            break;
        case NONETYPE:
            printf("NONETYPE");
            break;
    }
    if (node->rhs != NULL) {
        print_exp(node->rhs);
    }
}

int eval_exp(Node* node) {
    switch (node->type) {
        case INTEGER_E:
            return node->value;
            break;
        case ADDITION_E:
            return eval_exp(node->lhs) + eval_exp(node->rhs);
            break;
        case MULTIPLICATION_E:
            return eval_exp(node->lhs) * eval_exp(node->rhs);
            break;
        case OPEN_PARENTHESIS_E:
            return eval_exp(node->rhs);
            break;
        case CLOSE_PARENTHESIS_E:
            break;
        case NONETYPE:
            return eval_exp(node->rhs);
            break;
    }

    return -1;
}
*/

void enum_to_str(char* buf, TokenType type) {
    switch (type) {
        case KEYWORD:
            strncpy(buf, "KEYWORD", 8);
            break;
        case IDENTIFIER:
            strncpy(buf, "IDENTIFIER", 11);
            break;
        case EQUALS: 
            strncpy(buf, "EQUALS", 7);
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
