#ifndef PARSER_H
#define PARSER_H
#include <stdint.h>
#include "lexer.h"

typedef enum NodeType {
    NONETYPE,
    VARIABLE_DECLARATION_S,
    VARIABLE_E,
    ASSIGNMENT_E,
    OPEN_PARENTHESIS_E,
    CLOSE_PARENTHESIS_E,
    INTEGER_E,
    ADDITION_E,
    MULTIPLICATION_E,
    LOGICAL_EQUALS_E,
    RETURN_S,
    FUNCTION_DECLARATION_H,
    FUNCTION_DEFINITION_H,
    IF_H,
    BLOCK,
} NodeType;

typedef enum DataType {
    VOID,
    INT,
    CHAR,
    FLOAT,
    DOUBLE,
    PTR,
} DataType;

struct Node;
typedef struct NodeList {
    struct Node** nodes;
    int capacity;
} NodeList;

typedef struct TypeNode {
    DataType datatype;
    struct TypeNode* points_to;
} TypeNode;

typedef struct Node {
    union {
        struct {
            struct Node* lhs;
            struct Node* rhs;
        };
        NodeList* nodes;
    };
    union {
        uint64_t value;
        char* name;
    };
    NodeType type;
    TypeNode* datatype;
    uint8_t precedence;
} Node;

Node* create_node();
void destroy_node(Node* node);
Node* parse_expression(TokenList* tokens);
Node* parse_statement(TokenList* tokens, int* i);
Node* parse_block(TokenList* tokens, int* i);
void initialize_node_list(NodeList* array, int length);
void destroy_node_list(NodeList* array);
void increase_node_list_capacity(NodeList* array, int new_capacity);

#endif
