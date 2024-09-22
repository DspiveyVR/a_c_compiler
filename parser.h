#ifndef PARSER_H
#define PARSER_H
#include <stdint.h>
#include "lexer.h"

typedef enum NodeType {
    NONETYPE,
    INTEGER,
    ADDITION,
    MULTIPLICATION,
} NodeType;

typedef struct Node {
    NodeType type;
    // Could end up being a string since i'll handle identifiers later
    uint64_t value;
    uint8_t precedence;
    struct Node* lhs;
    struct Node* rhs;
} Node;

Node* create_node();
void destroy_node(Node* node);
Node* parse_expression(TokenList* tokens);
#endif
