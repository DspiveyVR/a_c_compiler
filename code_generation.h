#ifndef CODEGEN_H
#define CODEGEN_H

#include "dynamic_string.h"
#include "parser.h"

void generate_assembly(String* asm_string, Node* ast);

#endif
