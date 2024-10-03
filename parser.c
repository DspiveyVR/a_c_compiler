#include <stdlib.h>
#include <stdint.h>
#include <string.h>
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
} DataType;

struct Node;
typedef struct NodeList {
    struct Node** nodes;
    int capacity;
} NodeList;

typedef struct Node {
    NodeType type;
    DataType datatype;
    union {
        uint64_t value;
        char* name;
    };
    uint8_t precedence;
    union {
        struct {
            struct Node* lhs;
            struct Node* rhs;
        };
        NodeList* nodes;
    };
} Node;

Node* create_node();
void destroy_node(Node* node);
Node* parse_expression(TokenList* tokens, int* i);
Node* parse_statement(TokenList* tokens, int* i);
NodeList* initialize_node_list(int capacity);
void increase_node_list_capacity(NodeList* array, int new_capacity);

Node* parse_block(TokenList* tokens, int* i) {
    Node* block = create_node();
    Node* header = NULL;
    block->type = BLOCK;
    /*NodeList left_bracket_return_stack;*/

    for (;*i < tokens->length; (*i)++) { 
        switch (tokens->tokens[*i]->type) {
            case LEFT_BRACKET:
                {
                block->nodes = initialize_node_list(5);
                int node_index = 0;
                if (header != NULL)  {
                    block->nodes->nodes[0] = header;
                    node_index++;
                }
                while (tokens->tokens[*i]->type != RIGHT_BRACKET) {
                    if (*i >= block->nodes->capacity) {
                        increase_node_list_capacity(block->nodes, 
                                                    block->nodes->capacity + 5);
                    }

                    (*i)++;
                    switch (tokens->tokens[*i]->type) {
                        case KEYWORD:
                            if (strncmp(tokens->tokens[*i]->value, "if", 2) == 0) {
                            } else {
                                block->nodes->nodes[node_index] = parse_statement(tokens, i);
                                break;
                            }
                        case LEFT_BRACKET:
                            block->nodes->nodes[node_index] = parse_block(tokens, i);
                            break;
                        default:
                            block->nodes->nodes[node_index] = parse_statement(tokens, i);
                            break;
                    }
                }
                }
                (*i)--;
                break;
            case KEYWORD:
                {
                if (strncmp(tokens->tokens[*i]->value, "if", 2) == 0) {
                    header = create_node();
                    header->type = IF_H;
                    (*i)++;
                    header->rhs = parse_expression(tokens, i);
                }
                }
                break;
            case IDENTIFIER:
                {
                // FIXME: lazy
                header = create_node();
                header->type = FUNCTION_DEFINITION_H;
                *i += 2;
                }
                break;
            case RIGHT_BRACKET:
                return block;
            default:
                break;
        }
    }

    return block;
}

Node* parse_statement(TokenList* tokens, int* i) {
    Node* statement = NULL;
    for (;*i < tokens->length; (*i)++) { 
        switch (tokens->tokens[*i]->type) {
            case KEYWORD:
                if (strncmp(tokens->tokens[*i]->value, "int", 3) == 0) {
                    statement = create_node();
                    // FIXME: this one will have to change a lot later.
                    statement->type = VARIABLE_DECLARATION_S;
                    statement->datatype = INT;
                    statement->name = (char*)malloc(50);
                    strncpy(statement->name, tokens->tokens[(*i) + 1]->value, 50);
                    *i += 1;
                } else if (strncmp(tokens->tokens[*i]->value, "return", 6) == 0) {
                    statement = create_node();
                    statement->type = RETURN_S;
                    (*i)++;
                    statement->rhs = parse_expression(tokens, i);
                }
                break;
            case NONE:
                break;
            case RIGHT_BRACKET:
                return statement;
            // This encompasses any statement which is just an expression,
            // including assignment.
            default:
                statement = parse_expression(tokens, i);
                break;
        }
    }
    return statement;
}

Node* parse_expression(TokenList* tokens, int* i) {
    /*
        "expression" is the full binary tree, it is usually not directly
        modified, except when the current node is the first node in an
        expression.
        focus_node helps keep track of order of operations by pointing to
        the previous operator in the expression which is compared to the next 
        operator to determine which one takes precedence.

        If current_node and focus_node are both binary expressions and 
        current_node has a higher precedence than focus_node ("higher" is
        actually lower), then current_node will copy focus_node->rhs into
        current_node.lhs and then current_node will become focus_node->rhs,
        then focus_node is moved down to focus_node->rhs.
        Otherwise, if current_node has a lower precedence than focus_node,
        then focus_node is copied into current_node.lhs and focus_node is
        reassigned to current_node.
    */
    Node* expression = create_node();
    Node* focus_node = expression;
    // TODO: will need a stack of these
    // When the parser encounters a closing parenthesis, this return address
    // is assigned so that subsequent operators can be compared against the
    // entire parenthetical expression with the assumption that this 
    // parenthetical expression has been fully parsed.
    // If left_paren_return is not NULL  
    // then left_paren_return is reassigned to NULL.
    Node* left_paren_return = NULL;
    for (;*i < tokens->length; (*i)++) { 
        Node current_node;
        current_node.lhs = NULL;
        current_node.rhs = NULL;

        switch (tokens->tokens[*i]->type) {
            case CONSTANT:
                if (expression->type == NONETYPE) {
                    expression->type = INTEGER_E;
                    expression->value = atoi(tokens->tokens[*i]->value);
                    expression->precedence = 16;
                    continue;
                }
                current_node.type = INTEGER_E;
                current_node.value = atoi(tokens->tokens[*i]->value);
                current_node.precedence = 16;
                // Integers always have the lowest precedence.
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                break;
            case IDENTIFIER:
                if (expression->type == NONETYPE) {
                    expression->type = VARIABLE_E;
                    expression->name = (char*)malloc(50);
                    strncpy(expression->name, tokens->tokens[*i]->value, 50);
                    expression->precedence = 16;
                    continue;
                }
                current_node.type = VARIABLE_E;
                current_node.name = (char*)malloc(50);
                strncpy(current_node.name, tokens->tokens[*i]->value, 50);
                current_node.precedence = 16;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                break;
            case PLUS:
                current_node.type = ADDITION_E;
                current_node.precedence = 4;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER_E) ||
                        (focus_node->type == VARIABLE_E)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
                } else if (left_paren_return != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *left_paren_return;
                    *focus_node = current_node;
                    left_paren_return = NULL;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                break;
            // TODO: will have to determine the context to differentiate 
            // between multiplication and dereference.
            case STAR:
                current_node.type = MULTIPLICATION_E;
                current_node.precedence = 3;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER_E) ||
                        (focus_node->type == VARIABLE_E)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
                } else if (left_paren_return != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *left_paren_return;
                    *focus_node = current_node;
                    left_paren_return = NULL;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                break;
            case EQUALS:
                current_node.type = ASSIGNMENT_E;
                current_node.precedence = 14;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER_E) ||
                        (focus_node->type == VARIABLE_E)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
                } else if (left_paren_return != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *left_paren_return;
                    *focus_node = current_node;
                    left_paren_return = NULL;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                break;
            case EQUALS_EQUALS:
                current_node.type = LOGICAL_EQUALS_E;
                current_node.precedence = 7;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER_E) ||
                        (focus_node->type == VARIABLE_E)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
                } else if (left_paren_return != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *left_paren_return;
                    *focus_node = current_node;
                    left_paren_return = NULL;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                break;
            case LEFT_PARENTHESIS:
                if (expression->type == NONETYPE) {
                    expression->type = OPEN_PARENTHESIS_E;
                    expression->precedence = 0;
                    left_paren_return = focus_node;
                    continue;
                }
                current_node.type = OPEN_PARENTHESIS_E;
                current_node.precedence = 0;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                focus_node = focus_node->rhs;
                left_paren_return = focus_node;
                break;
            case RIGHT_PARENTHESIS:
                current_node.type = CLOSE_PARENTHESIS_E;
                current_node.precedence = 0;
                focus_node->rhs->rhs = create_node();
                *(focus_node->rhs->rhs) = current_node;
                focus_node = left_paren_return;
                break;
            case SEMICOLON:
                break;
            case LEFT_BRACKET:
            case RIGHT_BRACKET:
                (*i)--;
            case NONE:
            default:
                return expression;
        }
    }
    return expression;
} 

Node* create_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NONETYPE;
    node->lhs = NULL;
    node->rhs = NULL;
    node->nodes = NULL;
    node->name = NULL;
    return node;
}

void destroy_node(Node* node) {
    if (node->lhs != NULL) {
        destroy_node(node->lhs);
    }
    if (node->rhs != NULL) {
        destroy_node(node->rhs);
    }
    if ((node->type == VARIABLE_E) ||
            (node->type == VARIABLE_DECLARATION_S)) {
        free(node->name);
    }
    free(node);
}

NodeList* initialize_node_list(int capacity) {
    NodeList* array = malloc(sizeof(NodeList*));
    array->nodes = malloc(sizeof(Node*) * capacity);
    array->capacity = capacity;

    for (int i = 0; i < capacity; i++) {
        array->nodes[i] = NULL;
    }
    
    return array;
}

void destroy_node_list(NodeList* array) {
    free(array->nodes);
}

void increase_node_list_capacity(NodeList* array, int new_capacity) {
    array->nodes = realloc((void*)(array->nodes), new_capacity * sizeof(Node*));
    for (int i = array->capacity; i < new_capacity; i++) {
        array->nodes[i] = NULL;
    }
    array->capacity = new_capacity;
}
