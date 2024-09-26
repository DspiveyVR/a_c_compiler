#include "dynamic_string.h"
#include "parser.h"

void generate_assembly(String* asm_string, Node* ast) {
    set_string_at(asm_string, "\t.globl main\n", 0, 13);
    set_string_at(asm_string, "main:\n", 13, 6);
    set_string_at(asm_string, "mov $2, %rax\n", 19, 13);
    set_string_at(asm_string, "ret\n", 32, 4);
} 

void unravel_expression(String* asm_string, Node* input_node) {

}
