#include "dynamic_string.h"
#include "parser.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Variable {
    char* name;
    TypeNode* datatype;
    uint8_t scope_level;
    uint32_t stack_index;
} Variable;

typedef struct VariableList {
    Variable** variables;
    uint32_t length;
    uint32_t capacity;
} VariableList;

typedef struct FunctionHeader {
    char* name;
    TypeNode* return_type;
    NodeList* parameters;
} FunctionHeader;

typedef struct FunctionList { 
    FunctionHeader** functions;
    uint32_t length;
    uint32_t capacity;
} FunctionList;

void unravel_block(String* asm_string, Node* input_node, 
                   uint32_t* str_i, uint8_t scope_level, 
                   uint32_t stack_top, VariableList* variables, 
                   FunctionList* functions);
void unravel_expression(String* asm_string, Node* input_node, uint32_t* str_i, 
                        VariableList* variables, FunctionList* functions);
VariableList* initialize_variable_list(int capacity);
void destroy_variable_list(VariableList* array);
void push_var(VariableList* array, Variable* var);
void pop_var(VariableList* array);
FunctionList* initialize_function_list(int capacity);
void destroy_function_list(FunctionList* array);
void push_func(FunctionList* array, FunctionHeader* function);
void pop_func(FunctionList* array);
void increase_function_list_capacity(FunctionList* array, int new_capacity);

void generate_assembly(String* asm_string, Node* ast) {
    uint32_t str_i = 0;
    VariableList* variables = initialize_variable_list(5);
    FunctionList* functions = initialize_function_list(5);
    unravel_block(asm_string, ast, &str_i, 0, 0, variables, functions);
    destroy_variable_list(variables); 
    destroy_function_list(functions);
}

void unravel_block(String* asm_string, Node* input_node, 
                   uint32_t* str_i, uint8_t scope_level, 
                   uint32_t stack_top, VariableList* variables, 
                   FunctionList* functions) {
    for (int i = 0; i < input_node->nodes->capacity && input_node->nodes->nodes[i] != NULL; i++) {
        switch (input_node->nodes->nodes[i]->type) {
            case VARIABLE_DECLARATION_S:
                {
                Variable* new_var = malloc(sizeof(Variable));
                new_var->name = input_node->nodes->nodes[i]->name;
                new_var->datatype = input_node->nodes->nodes[i]->datatype;
                new_var->scope_level = scope_level;
                push_var(variables, new_var);
                char asm_code[13 + new_var->datatype->size / 10 + 1];
                sprintf(asm_code, "sub $%d, %%rsp\n", new_var->datatype->size);
                stack_top += new_var->datatype->size;
                new_var->stack_index = stack_top;
                set_string_at(asm_string, asm_code, *str_i, 13 + new_var->datatype->size / 10);
                *str_i += 13 + new_var->datatype->size / 10;
                }
                break;
            case BLOCK:
                unravel_block(asm_string, input_node->nodes->nodes[i], str_i, 
                              scope_level + 1, stack_top, variables, functions);
                break;
            case FUNCTION_DEFINITION_H: 
                {
                FunctionHeader* new_function = malloc(sizeof(FunctionHeader));
                new_function->name = input_node->nodes->nodes[i]->name;
                new_function->return_type = input_node->nodes->nodes[i]->datatype;
                new_function->parameters = input_node->nodes->nodes[i]->nodes;
                push_func(functions, new_function);

                if (strncmp(input_node->nodes->nodes[i]->name, "main", 50) == 0) {
                    set_string_at(asm_string, "\t.globl main\n", *str_i, 13);
                    *str_i += 13;
                }
                char asm_code2[strlen(input_node->nodes->nodes[i]->name) + 3];
                sprintf(asm_code2, "%s:\n", input_node->nodes->nodes[i]->name);
                set_string_at(asm_string, asm_code2, *str_i, strlen(input_node->nodes->nodes[i]->name) + 2);
                *str_i += strlen(input_node->nodes->nodes[i]->name) + 2;
                set_string_at(asm_string, "push %rbp\n", *str_i, 10);
                *str_i += 10;
                set_string_at(asm_string, "mov %rsp, %rbp\n", *str_i, 15);
                *str_i += 15;

                // Creating local variables from arguments passed to the function.
                if (new_function->parameters == NULL) {
                    break;
                }
                // Finding the length of the nodelist because you're stupid
                // and didn't create a length field initially.
                uint8_t param_count = 0;
                NodeList* parameters = new_function->parameters;
                for (size_t j = 0; (j < parameters->capacity) && 
                    (parameters->nodes[j] != NULL); j++) {
                    param_count++;
                }

                char regs[6][3] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                for (size_t j = param_count; j > 0; j--) {
                    Variable* new_var = malloc(sizeof(Variable));
                    new_var->name = input_node->nodes->nodes[i]->name;
                    new_var->datatype = input_node->nodes->nodes[i]->datatype;
                    new_var->scope_level = scope_level;
                    push_var(variables, new_var);

                    if (param_count > 6) {
                        // FIXME: var references need to account for the negative stack index.
                        new_var->stack_index = -((param_count - 6) * 8);
                    } else {
                        // Declare variable.
                        char asm_code[13 + new_var->datatype->size / 10 + 1];
                        sprintf(asm_code, "sub $%d, %%rsp\n", new_var->datatype->size);
                        stack_top += new_var->datatype->size;
                        new_var->stack_index = stack_top;
                        set_string_at(asm_string, asm_code, *str_i, 13 + new_var->datatype->size / 10);
                        *str_i += 13 + new_var->datatype->size / 10;

                        // Assign variable.
                        char asm_code2[15 + strnlen(regs[j - 1], 3) + (new_var->stack_index / 10) + 1];
                        sprintf(asm_code2, "mov %%%s, -%d(%%rbp)\n", regs[j - 1], new_var->stack_index);
                        set_string_at(asm_string, asm_code2, *str_i, 15 + strnlen(regs[j - 1], 3) + (new_var->stack_index / 10));
                        *str_i = 15 + strnlen(regs[j - 1], 3) + (new_var->stack_index / 10);
                    }
                }
                }
                break;
            case RETURN_S:
                {
                // Generates the code for the expression being returned.
                if (input_node->nodes->nodes[i]->rhs != NULL) {
                    unravel_expression(asm_string, input_node->nodes->nodes[i]->rhs, 
                                       str_i, variables, functions);
                }

                set_string_at(asm_string, "mov %rbp, %rsp\n", *str_i, 15);
                *str_i += 15;
                set_string_at(asm_string, "pop %rbp\n", *str_i, 9);
                *str_i += 9;
                set_string_at(asm_string, "ret\n", *str_i, 4);
                *str_i += 4;
                }
                return;
            default:
                unravel_expression(asm_string, input_node->nodes->nodes[i], 
                                   str_i, variables, functions);
                break;
        }
    }
    uint32_t bytes_to_reclaim = 0;
    for (int i = variables->length - 1; 
            i >= 0 && variables->variables[i] != NULL && variables->variables[i]->scope_level == scope_level; i--) {
        bytes_to_reclaim += variables->variables[i]->datatype->size;
        pop_var(variables);
    }
    if (bytes_to_reclaim > 0) {
        char asm_code[13 + bytes_to_reclaim / 10 + 1];
        sprintf(asm_code, "add $%d, %%rsp\n", bytes_to_reclaim);
        set_string_at(asm_string, asm_code, *str_i, 13 + bytes_to_reclaim / 10);
        *str_i += 13 + bytes_to_reclaim / 10;
    }

    return;
}

void unravel_expression(String* asm_string, Node* input_node, 
                        uint32_t* str_i, VariableList* variables,
                        FunctionList* functions) {
    switch (input_node->type) {
        case FUNCTION_CALL_E:
            {
            /*for (size_t i = 0; i < functions->length; i++) {*/
            /*    if (strncmp(functions->functions[i]->name, input_node->name, 50) == 0) {*/
                    // Finding the length of the nodelist because you're stupid
                    // and didn't create a length field initially.
                    uint8_t arg_count = 0;
                    NodeList* args = input_node->nodes;
                    for (size_t j = 0; (j < args->capacity) && 
                        (args->nodes[j] != NULL); j++) {
                        arg_count++;
                    }

                    char regs[6][3] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
                    for (size_t j = arg_count; j > 0; j--) {
                        if (arg_count > 6) {
                            unravel_expression(asm_string, args->nodes[j - 1]->rhs, str_i, 
                                               variables, functions);
                            set_string_at(asm_string, "push %rax", *str_i, 9);
                            *str_i += 9;
                        } else {
                            unravel_expression(asm_string, args->nodes[j - 1]->rhs, str_i, 
                                               variables, functions);
                            char asm_code[11 + strnlen(regs[j - 1], 3) + 1];
                            sprintf(asm_code, "mov %%rax, %%%s", regs[j - 1]);
                            set_string_at(asm_string, asm_code, *str_i, 11 + strnlen(regs[j - 1], 3));
                            *str_i = 11 + strnlen(regs[j - 1], 3);
                        }
                /*    }*/
                /*}*/
            }
            char asm_code[6 + strnlen(input_node->name, 50) + 1];
            sprintf(asm_code, "call %s\n", input_node->name);
            set_string_at(asm_string, asm_code, *str_i, 6 + strnlen(input_node->name, 50));
            *str_i += 6 + strnlen(input_node->name, 50);

            }
            break;
        case VARIABLE_E:
            {
            uint32_t var_stack_index = 0;
            for (int i = 0; i < variables->length; i++) {
                if (strncmp(variables->variables[i]->name, input_node->name, 50) == 0) {
                    var_stack_index = variables->variables[i]->stack_index;
                }
            }
            char asm_code[19 + var_stack_index / 10 + 1];
            sprintf(asm_code, "mov -%d(%%rbp), %%rax\n", var_stack_index);
            set_string_at(asm_string, asm_code, *str_i, 19 + var_stack_index / 10);
            *str_i += (19 + var_stack_index / 10);
            }
            break;
        case ASSIGNMENT_E:
            {
            unravel_expression(asm_string, input_node->rhs, str_i, 
                               variables, functions);

            uint32_t derefs = 0;
            Node* lhs_var = input_node->lhs;
            while (lhs_var->type == DEREFERENCE_E) {
                lhs_var = lhs_var->rhs;
                derefs++;
            }
            uint32_t var_stack_index = 0;
            uint32_t var_size = 0;
            for (int i = 0; i < variables->length; i++) {
                if (strncmp(variables->variables[i]->name, lhs_var->name, 50) == 0) {
                    var_stack_index = variables->variables[i]->stack_index;
                    var_size = variables->variables[i]->datatype->size;
                }
            }

            if (derefs == 0) {
                char asm_code[19 + var_stack_index / 10 + 1];
                // FIXME: will have to account for other sizes as well.
                if (var_size == 8) {
                    sprintf(asm_code, "mov %%rax, -%d(%%rbp)\n", var_stack_index);
                } else {
                    sprintf(asm_code, "mov %%eax, -%d(%%rbp)\n", var_stack_index);
                }
                set_string_at(asm_string, asm_code, *str_i, 19 + var_stack_index / 10);
                *str_i += (19 + var_stack_index / 10);
            } else {
                char asm_code[19 + var_stack_index / 10 + 1];
                sprintf(asm_code, "mov -%d(%%rbp), %%rcx\n", var_stack_index);
                set_string_at(asm_string, asm_code, *str_i, 19 + var_stack_index / 10);
                *str_i += (19 + var_stack_index / 10);

                for (size_t i = derefs - 1; i > 0; i--) {
                    set_string_at(asm_string, "mov (%rcx), %rcx\n", *str_i, 17);
                    *str_i += 17;
                }
                set_string_at(asm_string, "mov %rax, (%rcx)\n", *str_i, 17);
                *str_i += 17;
            }
            }
            break;
        case ADDRESS_OF_E: 
            {
            for (int i = 0; i < variables->length; i++) {
                if (strncmp(variables->variables[i]->name, input_node->rhs->name, 50) == 0) {
                    char asm_code[19 + variables->variables[i]->stack_index / 10 + 1];
                    sprintf(asm_code, "lea -%d(%%rbp), %%rax\n", variables->variables[i]->stack_index);
                    set_string_at(asm_string, asm_code, *str_i, 19 + variables->variables[i]->stack_index / 10);
                    *str_i += 19 + variables->variables[i]->stack_index / 10;
                }
            }           
            }
            break;
        case DEREFERENCE_E:
            {
            unravel_expression(asm_string, input_node->rhs, str_i, 
                               variables, functions);

            set_string_at(asm_string, "mov (%rax), %rax\n", *str_i, 17);
            *str_i += 17;
            }
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
            unravel_expression(asm_string, input_node->lhs, str_i, 
                               variables, functions);
            char asm_code[11];
            sprintf(asm_code, "push %%rax\n");
            set_string_at(asm_string, asm_code, *str_i, 10);
            *str_i += 10;
            unravel_expression(asm_string, input_node->rhs, str_i, 
                               variables, functions);
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
            unravel_expression(asm_string, input_node->lhs, str_i, 
                               variables, functions);
            char asm_code[11];
            sprintf(asm_code, "push %%rax\n");
            set_string_at(asm_string, asm_code, *str_i, 10);
            *str_i += 10;
            unravel_expression(asm_string, input_node->rhs, str_i, 
                               variables, functions);
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
            unravel_expression(asm_string, input_node->rhs, str_i, 
                               variables, functions);
            break;
        case CLOSE_PARENTHESIS_E:
        case NONETYPE:
        default:
            break;
    }

    return;
}

FunctionList* initialize_function_list(int capacity) {
    FunctionList* array = malloc(sizeof(FunctionList));
    array->functions = malloc(sizeof(FunctionHeader*) * capacity);
    array->length = 0;
    array->capacity = capacity;

    for (int i = 0; i < capacity; i++) {
        array->functions[i] = NULL;
    }

    return array;
}

void destroy_function_list(FunctionList* array) {
    for (size_t i = 0; i < array->capacity; i++) {
        free(array->functions[i]);
    }

    free(array->functions);
    free(array);
}

void push_func(FunctionList* array, FunctionHeader* function) {
    array->functions[array->length] = function;
    (array->length)++;
}

void increase_function_list_capacity(FunctionList* array, int new_capacity) {
    array->functions = realloc((void*)(array->functions), new_capacity * sizeof(FunctionHeader*));
    for (int i = array->capacity; i < new_capacity; i++) {
        array->functions[i] = NULL;
    }
    array->capacity = new_capacity;
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

void destroy_variable_list(VariableList* array) {
    for (size_t i = 0; i < array->capacity; i++) {
        free(array->variables[i]);
    }

    free(array->variables);
    free(array);
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
