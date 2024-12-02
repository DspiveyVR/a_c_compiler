#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "lexer.h"
#include <stdio.h>

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
Node* parse_expression(TokenList* tokens, int* i);
TypeNode* parse_datatype(TokenList* tokens, int* i);
void destroy_datatype(TypeNode* type);
Node* parse_statement(TokenList* tokens, int* i);
NodeList* initialize_node_list(int capacity);
void destroy_node_list(NodeList* array);
void increase_node_list_capacity(NodeList* array, int new_capacity);

Node* parse_block(TokenList* tokens, int* i) {
    Node* block = create_node();
    Node* header = NULL;
    block->type = BLOCK;

    while (*i < tokens->length) { 
        switch (tokens->tokens[*i]->type) {
            case LEFT_BRACKET:
                {
                block->nodes = initialize_node_list(5);
                int node_index = 0;
                if (header != NULL)  {
                    block->nodes->nodes[0] = header;
                    node_index++;
                }

                (*i)++;
                while (tokens->tokens[*i]->type != RIGHT_BRACKET) {
                    if (node_index >= block->nodes->capacity) {
                        increase_node_list_capacity(block->nodes, block->nodes->capacity + 5);
                    }

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
                    node_index++;
                }
                }
                break;
            case KEYWORD:
                {
                if (strncmp(tokens->tokens[*i]->value, "if", 2) == 0) {
                    header = create_node();
                    header->type = IF_H;
                    (*i)++;
                    header->rhs = parse_expression(tokens, i);
                } else if (strncmp(tokens->tokens[*i]->value, "int", 3) == 0) {
                    header = create_node();
                    header->type = FUNCTION_DEFINITION_H;
                    header->datatype = parse_datatype(tokens, i);
                    header->name = calloc(50, 1);
                    strncpy(header->name, tokens->tokens[*i]->value, 50);
                    *i += 2;
                    if (tokens->tokens[*i]->type != RIGHT_PARENTHESIS) {
                        header->nodes = initialize_node_list(5);
                        int node_index = 0;
                        if (node_index >= header->nodes->capacity) {
                            increase_node_list_capacity(block->nodes, block->nodes->capacity + 5);
                        }
                        while (tokens->tokens[*i]->type != RIGHT_PARENTHESIS) {
                            header->nodes->nodes[node_index] = parse_statement(tokens, i);                           
                            node_index++;
                        }
                    }
                    (*i)++;
                }
                }
                break;
            case RIGHT_BRACKET:
                (*i)++;
                return block;
            default:
                (*i)++;
                break;
        }
    }

    return block;
}

Node* parse_statement(TokenList* tokens, int* i) {
    Node* statement = NULL;
    while (*i < tokens->length) { 
        switch (tokens->tokens[*i]->type) {
            case KEYWORD:
                if ((strncmp(tokens->tokens[*i]->value, "int", 3) == 0) ||
                     (strncmp(tokens->tokens[*i]->value, "char", 4) == 0) ||
                     (strncmp(tokens->tokens[*i]->value, "float", 5) == 0) ||
                     (strncmp(tokens->tokens[*i]->value, "double", 6) == 0) ||
                     (strncmp(tokens->tokens[*i]->value, "void", 4) == 0)) {
                    statement = create_node();
                    statement->type = VARIABLE_DECLARATION_S;
                    statement->datatype = parse_datatype(tokens, i);
                    statement->name = malloc(50);
                    strncpy(statement->name, tokens->tokens[*i]->value, 50);
                    (*i)++;
                } else if (strncmp(tokens->tokens[*i]->value, "return", 6) == 0) {
                    statement = create_node();
                    statement->type = RETURN_S;
                    (*i)++;
                    statement->rhs = parse_expression(tokens, i);
                }
                break;
            case SEMICOLON:
            case COMMA:
                (*i)++;
                return statement;
            case NONE:
            case LEFT_BRACKET:
            case RIGHT_BRACKET:
            case RIGHT_PARENTHESIS:
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

TypeNode* parse_datatype(TokenList* tokens, int* i) {
    TypeNode* type = malloc(sizeof(TypeNode));
    type->points_to = NULL;
    TypeNode* focus_node = type;
    for (; tokens->tokens[*i]->type != IDENTIFIER; (*i)++);
    int j = *i - 1;

    while (tokens->tokens[j]->type == STAR) {
        focus_node->datatype = PTR;
        focus_node->size = 8;
        focus_node->points_to = malloc(sizeof(TypeNode));
        focus_node = focus_node->points_to;
        focus_node->points_to = NULL;
        j--;
    }

    // It should always be a keyword.
    if (strncmp(tokens->tokens[j]->value, "int", 3) == 0) {
        focus_node->datatype = INT;
        focus_node->size = 4;
    } else if (strncmp(tokens->tokens[j]->value, "char", 4) == 0) {
        focus_node->datatype = CHAR;
        focus_node->size = 1;
    } else if (strncmp(tokens->tokens[j]->value, "float", 5) == 0) {
        focus_node->datatype = FLOAT;
        focus_node->size = 4;
    } else if (strncmp(tokens->tokens[j]->value, "double", 6) == 0) {
        focus_node->datatype = DOUBLE;
        focus_node->size = 8;
    } else if (strncmp(tokens->tokens[j]->value, "void", 4) == 0) {
        focus_node->datatype = VOID;
    }


    return type;
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
    /*
        When the parser encounters a closing parenthesis, this return address
        is assigned so that subsequent operators can be compared against the
        entire parenthetical expression with the assumption that this 
        parenthetical expression has been fully parsed.
        If left_paren_return is not NULL  
        then left_paren_return is reassigned to NULL.
    */
    int return_stack_top = 0;
    // Let's be real you don't need more than this.
    Node* left_paren_return_stack[10] = {NULL};
    /*
        This allows multiple unary operators to work.  After the full 
        sequence of unary operators is parsed, the first one becomes the focus
        node, so that when a binary operator steals its operand, it will take
        the entire operand including all of the unary operators.
    */
    Node* unary_operator_return = NULL;
    while (*i < tokens->length) { 
        Node current_node;
        current_node.lhs = NULL;
        current_node.rhs = NULL;

        switch (tokens->tokens[*i]->type) {
            case CONSTANT:
                if (expression->type == NONETYPE) {
                    expression->type = INTEGER_E;
                    expression->value = atoi(tokens->tokens[*i]->value);
                    expression->precedence = 16;
                    (*i)++;
                    continue;
                }
                current_node.type = INTEGER_E;
                current_node.value = atoi(tokens->tokens[*i]->value);
                current_node.precedence = 16;
                // Integers always have the lowest precedence.
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                if (unary_operator_return != NULL) {
                    focus_node = unary_operator_return;
                    unary_operator_return = NULL;
                }

                (*i)++;
                break;
            case IDENTIFIER:
                if (tokens->tokens[*i + 1]->type == LEFT_PARENTHESIS) {
                    if (expression->type == NONETYPE) { 
                        expression->type = FUNCTION_CALL_E;
                        expression->name = malloc(50);
                        expression->precedence = 16;
                        strncpy(expression->name, tokens->tokens[*i]->value, 50);
                        expression->nodes = initialize_node_list(5);
                        (*i)++;
                        continue;
                    }
                    current_node.type = FUNCTION_CALL_E;
                    current_node.name = malloc(50);
                    current_node.precedence = 16;
                    strncpy(current_node.name, tokens->tokens[*i]->value, 50);
                    current_node.nodes = initialize_node_list(5);
                    focus_node->rhs = create_node();
                    *(focus_node->rhs) = current_node;
                    if (unary_operator_return != NULL) {
                        focus_node = unary_operator_return;
                        unary_operator_return = NULL;
                    } else {
                        focus_node = focus_node->rhs;
                    }
                    (*i)++;
                    continue;
                }
                if (expression->type == NONETYPE) {
                    expression->type = VARIABLE_E;
                    expression->name = malloc(50);
                    strncpy(expression->name, tokens->tokens[*i]->value, 50);
                    expression->precedence = 16;
                    (*i)++;
                    continue;
                }
                current_node.type = VARIABLE_E;
                current_node.name = malloc(50);
                strncpy(current_node.name, tokens->tokens[*i]->value, 50);
                current_node.precedence = 16;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                if (unary_operator_return != NULL) {
                    focus_node = unary_operator_return;
                    unary_operator_return = NULL;
                } 

                (*i)++;
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
                } else if (left_paren_return_stack[return_stack_top] != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = 
                                    *(left_paren_return_stack[return_stack_top]);
                    left_paren_return_stack[return_stack_top] = NULL;
                    *focus_node = current_node;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                (*i)++;
                break;
            case STAR:
                if ((tokens->tokens[*i - 1]->type == CONSTANT) ||
                        (tokens->tokens[*i - 1]->type == IDENTIFIER) ||
                        (tokens->tokens[*i - 1]->type == RIGHT_PARENTHESIS)) {
                    current_node.type = MULTIPLICATION_E;
                    current_node.precedence = 3;
                    if ((current_node.precedence > focus_node->precedence) ||
                            (focus_node->type == INTEGER_E) ||
                            (focus_node->type == VARIABLE_E)) {
                        current_node.lhs = create_node();
                        *(current_node.lhs) = *focus_node;
                        *focus_node = current_node;
                    } else if (left_paren_return_stack[return_stack_top] != NULL) {
                        current_node.lhs = create_node();
                        *(current_node.lhs) = 
                            *(left_paren_return_stack[return_stack_top]);
                        left_paren_return_stack[return_stack_top] = NULL;
                        *focus_node = current_node;
                    } else {
                        current_node.lhs = create_node();
                        *(current_node.lhs) = *(focus_node->rhs);
                        *(focus_node->rhs) = current_node;
                        focus_node = focus_node->rhs;
                    }
                } else {
                    if (expression->type == NONETYPE) {
                        expression->type = DEREFERENCE_E;
                        expression->precedence = 2;
                        if (unary_operator_return == NULL) {
                            unary_operator_return = expression;
                        }
                        (*i)++;
                        continue;
                    }
                    current_node.type = DEREFERENCE_E;
                    current_node.precedence = 2;
                    focus_node->rhs = create_node();
                    *(focus_node->rhs) = current_node;
                    if (unary_operator_return == NULL) {
                        unary_operator_return = focus_node;
                    }
                    focus_node = focus_node->rhs;
                }
                (*i)++;
                break;
            case AMPERSAND:
                if (expression->type == NONETYPE) {
                    expression->type = ADDRESS_OF_E;
                    expression->precedence = 2;
                    (*i)++;
                    continue;
                }
                current_node.type = ADDRESS_OF_E;
                current_node.precedence = 2;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                focus_node = focus_node->rhs;
                (*i)++;
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
                } else if (left_paren_return_stack[return_stack_top] != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = 
                                    *(left_paren_return_stack[return_stack_top]);
                    left_paren_return_stack[return_stack_top] = NULL;
                    *focus_node = current_node;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                (*i)++;
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
                } else if (left_paren_return_stack[return_stack_top] != NULL) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = 
                                    *(left_paren_return_stack[return_stack_top]);
                    left_paren_return_stack[return_stack_top] = NULL;
                    *focus_node = current_node;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                (*i)++;
                break;
            case LEFT_PARENTHESIS:
                if (expression->type == NONETYPE) {
                    expression->type = OPEN_PARENTHESIS_E;
                    expression->precedence = 0;
                    left_paren_return_stack[return_stack_top] = focus_node;
                    return_stack_top++;
                    (*i)++;
                    continue;
                }
                current_node.type = OPEN_PARENTHESIS_E;
                current_node.precedence = 0;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                left_paren_return_stack[return_stack_top] = focus_node;
                return_stack_top++;
                if (focus_node->type == FUNCTION_CALL_E && tokens->tokens[*i + 1]->type != RIGHT_PARENTHESIS) {
                    focus_node->nodes->nodes[0] = create_node();
                    focus_node->nodes->nodes[0]->type = ARGUMENT_E;
                    focus_node->nodes->nodes[0]->precedence = 16;
                    focus_node = focus_node->nodes->nodes[0];
                } else {
                    focus_node = focus_node->rhs;
                }
                (*i)++;
                break;
            case RIGHT_PARENTHESIS:
                current_node.type = CLOSE_PARENTHESIS_E;
                current_node.precedence = 0;
                // Checks how many layers deep the parentheses are.
                if (tokens->tokens[*i - 1]->type != LEFT_PARENTHESIS) {
                    Node** closeptr = &(focus_node->rhs->rhs);
                    while (*closeptr != NULL) {
                        closeptr = &((**closeptr).rhs);
                    }
                    *closeptr = create_node();
                    **closeptr = current_node;
                }
                return_stack_top--;
                focus_node = left_paren_return_stack[return_stack_top];
                (*i)++;
                break;
            case COMMA:
                if (tokens->tokens[*i + 1]->type != RIGHT_PARENTHESIS) {
                    for (int node_index = 0; node_index < left_paren_return_stack[return_stack_top - 1]->nodes->capacity; node_index++) {
                        if (left_paren_return_stack[return_stack_top - 1]->nodes->nodes[node_index] == NULL) {
                            left_paren_return_stack[return_stack_top - 1]->nodes->nodes[node_index] = create_node();
                            left_paren_return_stack[return_stack_top - 1]->nodes->nodes[node_index]->type = ARGUMENT_E;
                            left_paren_return_stack[return_stack_top - 1]->nodes->nodes[node_index]->precedence = 16;
                            focus_node = left_paren_return_stack[return_stack_top - 1]->nodes->nodes[node_index];
                            break;
                        }
                    }
                }
                (*i)++;
                break;
            case SEMICOLON:
            case LEFT_BRACKET:
            case RIGHT_BRACKET:
            case NONE:
            default:
                return expression;
        }
    }
    return expression;
} 

Node* create_node() {
    Node* node = malloc(sizeof(Node));
    node->type = NONETYPE;
    node->lhs = NULL;
    node->rhs = NULL;
    node->nodes = NULL;
    node->name = NULL;
    return node;
}

void destroy_node(Node* node) {
    if (node->type == VARIABLE_E) {
        free(node->name);
    } if(node->type == VARIABLE_DECLARATION_S) {
        destroy_datatype(node->datatype);
        free(node->name);
    } else if (node->type == BLOCK) {
        destroy_node_list(node->nodes);
        free(node);
        return;
    } else if (node->type == FUNCTION_DEFINITION_H) {
        if (node->nodes != NULL) {
            destroy_node_list(node->nodes);
        }
        destroy_datatype(node->datatype);
        free(node->name);
        free(node);
        return;
    } else if (node->type == FUNCTION_CALL_E) {
        if (node->nodes != NULL) {
            destroy_node_list(node->nodes);
        }
        free(node->name);
        free(node);
        return;
    }

    if (node->lhs != NULL) {
        destroy_node(node->lhs);
    }
    if (node->rhs != NULL) {
        destroy_node(node->rhs);
    }
    free(node);
}

void destroy_datatype(TypeNode* type) {
    if (type->points_to != NULL) {
        destroy_datatype(type->points_to);
    }

    free(type);
}

NodeList* initialize_node_list(int capacity) {
    NodeList* array = malloc(sizeof(NodeList));
    array->nodes = malloc(sizeof(Node*) * capacity);
    array->capacity = capacity;

    for (int i = 0; i < capacity; i++) {
        array->nodes[i] = NULL;
    }
    
    return array;
}

void destroy_node_list(NodeList* array) {
    for (int i = 0; (i < array->capacity) && (array->nodes[i] != NULL); i++) {
        destroy_node(array->nodes[i]);
    }
    free(array->nodes);
    free(array);
}

void increase_node_list_capacity(NodeList* array, int new_capacity) {
    array->nodes = realloc((void*)(array->nodes), new_capacity * sizeof(Node*));
    for (int i = array->capacity; i < new_capacity; i++) {
        array->nodes[i] = NULL;
    }
    array->capacity = new_capacity;
}
