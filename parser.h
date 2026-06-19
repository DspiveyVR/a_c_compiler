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
    DEREFERENCE_E,
    ADDRESS_OF_E,
    LOGICAL_EQUALS_E,
    RETURN_S,
    FUNCTION_DECLARATION_H,
    FUNCTION_DEFINITION_H,
    FUNCTION_CALL_E,
    // Well well well, if it isn't the consequences of my own actions.
    ARGUMENT_E,
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
    STRUCT
} DataType;

struct Node;
typedef struct NodeList {
    struct Node** nodes;
    int capacity;
} NodeList;

typedef struct TypeNode {
    DataType datatype;
    uint8_t size;
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
Node* parse(TokenList* tokens, int* i);
NodeList* initialize_node_list(int capacity);
void destroy_node_list(NodeList* array);
void increase_node_list_capacity(NodeList* array, int new_capacity);

#endif
