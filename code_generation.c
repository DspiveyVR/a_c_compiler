#include "dynamic_string.h"
#include "parser.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Variable {
    char* name;
    TypeNode* datatype;
    uint8_t scope_level;
    uint32_t stack_index;
    char* register_name;
} Variable;

typedef struct VariableList {
    Variable** variables;
    uint32_t length;
    uint32_t capacity;
} VariableList;

void unravel_block(String* asm_string, Node* input_node, 
                        uint32_t* str_i, uint8_t scope_level, 
                        uint32_t stack_top, VariableList* variables);
void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i);
VariableList* initialize_variable_list(int capacity);
void push_var(VariableList* array, Variable* var);
void pop_var(VariableList* array);

void generate_assembly(String* asm_string, Node* ast) {
    /*set_string_at(asm_string, "\t.globl main\n", 0, 13);*/
    /*set_string_at(asm_string, "main:\n", 13, 6);*/
    /*uint32_t str_i = 19;*/
    /*unravel_expression(asm_string, ast, &str_i);*/
    /*set_string_at(asm_string, "ret\n", str_i, 4);*/
    uint32_t str_i = 0;
    VariableList* variables = initialize_variable_list(5);
    unravel_block(asm_string, ast, &str_i, 0, 0, variables);
}

void unravel_block(String* asm_string, Node* input_node, 
                        uint32_t* str_i, uint8_t scope_level, 
                        uint32_t stack_top, VariableList* variables) {
    for (int i = 0; i < input_node->nodes->capacity && input_node->nodes->nodes[i] != NULL; i++) {
        switch (input_node->nodes->nodes[i]->type) {
            case VARIABLE_DECLARATION_S:
                {
                Variable* new_var = malloc(sizeof(Variable));
                new_var->name = input_node->nodes->nodes[i]->name;
                new_var->datatype = input_node->nodes->nodes[i]->datatype;
                new_var->scope_level = scope_level;
                new_var->stack_index = stack_top;
                push_var(variables, new_var);
                char asm_code[13 + new_var->datatype->size / 10 + 1];
                sprintf(asm_code, "sub $%d, %%rsp\n", new_var->datatype->size);
                stack_top += new_var->datatype->size;
                set_string_at(asm_string, asm_code, *str_i, 13 + new_var->datatype->size / 10);
                *str_i += 13 + new_var->datatype->size / 10;
                }
                break;
            case BLOCK:
                unravel_block(asm_string, input_node->nodes->nodes[i], str_i, scope_level + 1, stack_top, variables);
                break;
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
                set_string_at(asm_string, "push %rbp\n", *str_i, 10);
                stack_top += 8;
                *str_i += 10;
                set_string_at(asm_string, "mov %rsp, %rbp\n", *str_i, 15);
                *str_i += 15;
                }
                break;
            case RETURN_S:
                {
                uint32_t bytes_to_reclaim = 0;
                for (int i = variables->length - 1; 
                        i >= 0 && variables->variables[i] != NULL && variables->variables[i]->scope_level == scope_level; i--) {
                    bytes_to_reclaim += variables->variables[i]->datatype->size;
                    pop_var(variables);
                }
                char asm_code[13 + bytes_to_reclaim / 10 + 1];
                sprintf(asm_code, "add $%d, %%rsp\n", bytes_to_reclaim);
                set_string_at(asm_string, asm_code, *str_i, 13 + bytes_to_reclaim / 10);
                *str_i += 13 + bytes_to_reclaim / 10;
                set_string_at(asm_string, "pop %rbp\n", *str_i, 9);
                *str_i += 9;

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
                return;
            default:
                break;
        }
    }

    uint32_t bytes_to_reclaim = 0;
    for (int i = variables->length - 1; 
            i >= 0 && variables->variables[i] != NULL && variables->variables[i]->scope_level == scope_level; i--) {
        bytes_to_reclaim += variables->variables[i]->datatype->size;
        pop_var(variables);
    }
    char asm_code[13 + bytes_to_reclaim / 10 + 1];

    return;
}

void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i) {
    switch (input_node->type) {
        case VARIABLE_E:

            break;
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

VariableList* initialize_variable_list(int capacity) {
    VariableList* array = malloc(sizeof(VariableList));
    array->variables = malloc(sizeof(Variable*) * capacity);
    array->length = 0;
    array->capacity = capacity;

    for (int i = 0; i < capacity; i++) {
        array->variables[i] = NULL;
    }

    return array;
}

void push_var(VariableList* array, Variable* var) {
    array->variables[array->length] = var;
    (array->length)++;
}

void pop_var(VariableList* array) {
    free(array->variables[array->length - 1]);
    (array->length)--;
}

void increase_variable_list_capacity(VariableList* array, int new_capacity) {
    array->variables = realloc((void*)(array->variables), new_capacity * sizeof(Variable*));
    for (int i = array->capacity; i < new_capacity; i++) {
        array->variables[i] = NULL;
    }
    array->capacity = new_capacity;
}
