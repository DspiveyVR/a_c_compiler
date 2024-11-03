#include "dynamic_string.h"
#include "parser.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void unravel_block(String* asm_string, Node* input_node, uint32_t* str_i);
void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i);

void generate_assembly(String* asm_string, Node* ast) {
    /*set_string_at(asm_string, "\t.globl main\n", 0, 13);*/
    /*set_string_at(asm_string, "main:\n", 13, 6);*/
    /*uint32_t str_i = 19;*/
    /*unravel_expression(asm_string, ast, &str_i);*/
    /*set_string_at(asm_string, "ret\n", str_i, 4);*/
    uint32_t str_i = 0;
    unravel_block(asm_string, ast->nodes->nodes[0], &str_i);
}

void unravel_block(String* asm_string, Node* input_node, uint32_t* str_i) {
    for (int i = 0; i < input_node->nodes->capacity && input_node->nodes->nodes[i] != NULL; i++) {
        switch (input_node->nodes->nodes[i]->type) {
            case FUNCTION_DEFINITION_H: 
                {
                if (strncmp(input_node->nodes->nodes[i]->name, "main", 50) == 0) {
                    set_string_at(asm_string, "\t.globl main\n", *str_i, 13);
                    *str_i += 13;
                }
                char asm_code2[strlen(input_node->nodes->nodes[i]->name) + 3];
                sprintf(asm_code2, "%s:\n", input_node->nodes->nodes[i]->name);
                set_string_at(asm_string, asm_code2, *str_i, strlen(input_node->nodes->nodes[i]->name) + 2);
                *str_i += strlen(input_node->nodes->nodes[i]->name) + 2;
                }
                break;
            case RETURN_S:
                {
                if (input_node->nodes->nodes[i]->rhs != NULL) {
                    String expression_code;
                    initialize_string(&expression_code, 5);
                    uint32_t exp_str_i = 0;
                    unravel_expression(&expression_code, input_node->nodes->nodes[i]->rhs, &exp_str_i);
                    uint32_t code_len = strnlen(expression_code.string, expression_code.length);
                    set_string_at(asm_string, expression_code.string, *str_i, code_len);
                    *str_i += code_len;
                    destroy_string(&expression_code);
                }
                set_string_at(asm_string, "ret\n", *str_i, 4);
                *str_i += 4;
                }
                break;
            default:
                break;
        }
    }
}

void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i) {
    switch (input_node->type) {
        case INTEGER_E:
            {
            char asm_code[13 + input_node->value / 10 + 1];
            sprintf(asm_code, "mov $%d, %%rax\n", (uint32_t)input_node->value);
            set_string_at(asm_string, asm_code, *str_i, 13 + input_node->value / 10);
            *str_i += (13 + input_node->value / 10);
            }
            break;
        case ADDITION_E:
            {
            unravel_expression(asm_string, input_node->lhs, str_i);
            char asm_code[11];
            sprintf(asm_code, "push %%rax\n");
            set_string_at(asm_string, asm_code, *str_i, 10);
            *str_i += 10;
            unravel_expression(asm_string, input_node->rhs, str_i);
            char asm_code2[10];
            sprintf(asm_code2, "pop %%rcx\n");
            char asm_code3[16];
            sprintf(asm_code3, "add %%rcx, %%rax\n");
            set_string_at(asm_string, asm_code2, *str_i, 9);
            *str_i += 9;
            set_string_at(asm_string, asm_code3, *str_i, 15);
            *str_i += 15;
            }
            break;
        case MULTIPLICATION_E:
            {
            unravel_expression(asm_string, input_node->lhs, str_i);
            char asm_code[11];
            sprintf(asm_code, "push %%rax\n");
            set_string_at(asm_string, asm_code, *str_i, 10);
            *str_i += 10;
            unravel_expression(asm_string, input_node->rhs, str_i);
            char asm_code2[10];
            sprintf(asm_code2, "pop %%rcx\n");
            char asm_code3[17];
            sprintf(asm_code3, "imul %%rcx, %%rax\n");
            set_string_at(asm_string, asm_code2, *str_i, 9);
            *str_i += 9;
            set_string_at(asm_string, asm_code3, *str_i, 16);
            *str_i += 16;
            }
            break;
        case OPEN_PARENTHESIS_E:
            unravel_expression(asm_string, input_node->rhs, str_i);
            break;
        case CLOSE_PARENTHESIS_E:
        case NONETYPE:
        default:
            break;
    }

    return;
}
