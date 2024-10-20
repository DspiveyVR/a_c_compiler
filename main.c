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
void print_ast(Node* node, int indentation);
void print_datatype(TypeNode* type);

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
    printf("\n\n");
    for (int i = 0; i < tokens.length; i++) {
        // strncpy(tokens[i].value, "int", 3);
        char type[20] = "";
        enum_to_str(type, tokens.tokens[i]->type);

        printf("  %s %s\n", type, tokens.tokens[i]->value);
    }

    int token_index = 0;
    Node* block = parse_block(&tokens, &token_index);
    printf("\n");
    bg_magenta();
    bold();
    printf("AST");
    clear();
    printf("\n");
    print_ast(block, 0);
    destroy_token_list(&tokens);
    destroy_node(block);



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

void print_ast(Node* node, int indentation) {
    if (node == NULL) {
        return;
    }
    char indentation_str[16] = "";
    memset(indentation_str, ' ', indentation);

    switch (node->type) {
        case BLOCK:
            printf("\n");
            printf("%s", indentation_str);
            printf("BLOCK {\n");
            for (int i = 0; node->nodes->nodes[i] != NULL; i++) {
                switch (node->nodes->nodes[i]->type) {
                    case BLOCK:
                        print_ast(node->nodes->nodes[i], indentation + 2);
                        break;
                    default:
                        {
                        char indentation_str[16] = "";
                        memset(indentation_str, ' ', indentation + 2);
                        printf("%s", indentation_str);
                        print_ast(node->nodes->nodes[i], indentation + 2);
                        }
                        break;
                }
                printf("\n");
            }
            printf("%s", indentation_str);
            printf("}\n");
            break;
        case IF_H:
            clear();
            printf("IF(");
            print_ast(node->rhs, indentation);
            clear();
            printf(")");
            break;
        case FUNCTION_DEFINITION_H:
            clear();
            printf("FUNCTION_DEFINITION_H(");
            // FIXME: more datatypes later.
            printf("(");
            print_datatype(node->datatype);
            printf(", \"%s\")", node->name);
            if (node->nodes != NULL) {
                printf(", ");
                for (int i = 0; node->nodes->nodes[i] != NULL; i++) {
                    print_ast(node->nodes->nodes[i], indentation);
                    if (node->nodes->nodes[i + 1] != NULL) {
                        printf(", ");
                    }
                }
            }
            clear();
            printf(")");
            break;
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
            print_ast(node->lhs, indentation);
            clear();
            fg_cyan();
            dim();
            printf(",");
            print_ast(node->rhs, indentation);
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
            print_ast(node->lhs, indentation);
            clear();
            fg_bright_red();
            dim();
            printf(",");
            print_ast(node->rhs, indentation);
            clear();
            fg_bright_red();
            dim();
            printf(")");
            break;
        case LOGICAL_EQUALS_E:
            clear();
            fg_bright_red();
            bold();
            printf("L_EQ");
            clear_bold();
            dim();
            printf("(");
            print_ast(node->lhs, indentation);
            clear();
            fg_bright_red();
            dim();
            printf(",");
            print_ast(node->rhs, indentation);
            clear();
            fg_bright_red();
            dim();
            printf(")");
            break;
        case OPEN_PARENTHESIS_E:
            print_ast(node->rhs, indentation);
            break;
        case CLOSE_PARENTHESIS_E:
            break;
        case VARIABLE_DECLARATION_S:
            clear();           
            // INT is only type so it's hardcoded for now.
            printf("VAR_DECLARATION(");
            print_datatype(node->datatype);
            printf(", %s)", node->name);
            break;
        case ASSIGNMENT_E:
            clear();
            printf("ASSIGNMENT(");
            print_ast(node->lhs, indentation);
            clear();
            printf(",");
            print_ast(node->rhs, indentation);
            clear();
            printf(")");
            break;
        case RETURN_S:
            clear();
            printf("RETURN(");
            print_ast(node->rhs, indentation);
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

void print_datatype(TypeNode* type) {
    switch (type->datatype) {
        case INT:
            printf("INT");
            break;
        case CHAR:
            printf("CHAR");
            break;       
        case FLOAT:
            printf("FLOAT");
            break;
        case DOUBLE:
            printf("DOUBLE");
            break;
        case VOID:
            printf("VOID");
            break;
        case PTR:
            printf("PTR(");
            print_datatype(type->points_to);
            printf(")");
            break;
        default: 
            break;
    }
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
        case COMMA: 
            strncpy(buf, "COMMA", 6);
            break;
        case EQUALS: 
            strncpy(buf, "EQUALS", 7);
            break;
        case EQUALS_EQUALS: 
            strncpy(buf, "EQUALS_EQUALS", 14);
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
