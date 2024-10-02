#include "dynamic_string.h"
#include "parser.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i);

void generate_assembly(String* asm_string, Node* ast) {
    set_string_at(asm_string, "\t.globl main\n", 0, 13);
    set_string_at(asm_string, "main:\n", 13, 6);
    uint32_t str_i = 19;
    unravel_expression(asm_string, ast, &str_i);
    /*set_string_at(asm_string, "mov $2, %rax\n", 19, 13);*/
    set_string_at(asm_string, "ret\n", str_i, 4);
} 

void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i) {
    switch (input_node->type) {
        case INTEGER_E:
            {
            char asm_code[30];
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
            char asm_code2[25];
            sprintf(asm_code2, "pop %%rcx\n");
            sprintf(asm_code2 + 9, "add %%rcx, %%rax\n");
            set_string_at(asm_string, asm_code2, *str_i, 24);
            *str_i += 24;
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
            char asm_code2[26];
            sprintf(asm_code2, "pop %%rcx\n");
            sprintf(asm_code2 + 9, "imul %%rcx, %%rax\n");
            set_string_at(asm_string, asm_code2, *str_i, 25);
            *str_i += 25;
            }
            break;
        case OPEN_PARENTHESIS_E:
            unravel_expression(asm_string, input_node->rhs, str_i);
            break;
        case CLOSE_PARENTHESIS_E:
            break;
        case NONETYPE:
            break;
    }

    return;
}
