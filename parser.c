#include "colors.h"
#include "lexer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef enum DataType { VOID, INT, CHAR, FLOAT, DOUBLE, PTR, STRUCT } DataType;

struct Node;
typedef struct NodeList {
    struct Node** nodes;
    int capacity;
} NodeList;

typedef struct StructBody {
    char* name;
    NodeList fields;
} Struct;

typedef struct TypeNode {
    DataType datatype;
    uint8_t size;
    union {
        struct TypeNode* points_to;
        struct StructBody* struct_body;
    };
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
Node* parse_block(TokenList* tokens, int* i, Node* header);
Node* parse_control_flow_header(TokenList* tokens, int* i);
Node* parse_function_header(TokenList* tokens, int* i);

Node* parse(TokenList* tokens, int* i) {
    Node* root_block = create_node();
    root_block->type = BLOCK;
    int block_i = 0;
    // FIXME: Make resizable.
    root_block->nodes = initialize_node_list(10);
    Node* held_header = NULL;

    while (*i < tokens->length) {
        switch (tokens->tokens[*i]->type) {
        case KEYWORD:
            if (!strncmp(tokens->tokens[*i]->value, "int", 3)) {
                held_header = parse_function_header(tokens, i);
                if (held_header->type == FUNCTION_DECLARATION_H) {
                    root_block->nodes->nodes[block_i] = held_header;
                    held_header = NULL;
                    block_i++;
                } else if (!strncmp(tokens->tokens[*i]->value, "struct", 6)) {
                }
            }
            break;
        case LEFT_BRACKET:
            root_block->nodes->nodes[block_i] =
                parse_block(tokens, i, held_header);
            held_header = NULL;
            block_i++;
            break;
        case NONE:
            return root_block;
        default:
            break;
        }
    }

    return root_block;
}

Node* parse_block(TokenList* tokens, int* i, Node* function_header) {
    Node* root_block = NULL;
    Node* focus_block = NULL;
    int focus_block_i = 0;
    Node* held_header = NULL;
    // FIXME: all blocks need to have resizable lists.
    NodeList* parent_stack = initialize_node_list(10);
    int parent_indices[10] = {0};
    int parent_stack_top = 0;
    while (*i < tokens->length) {
        Node current_node = {0};
        switch (tokens->tokens[*i]->type) {
        case LEFT_BRACKET:
            (*i)++;
            if (root_block == NULL) {
                root_block = create_node();
                parent_stack->nodes[0] = root_block;
                root_block->type = BLOCK;
                root_block->nodes = initialize_node_list(10);
                root_block->nodes->nodes[0] = function_header;
                parent_indices[0]++;
                function_header = NULL;
                focus_block = root_block;
            } else {
                if (focus_block != root_block) {
                    parent_stack_top++;
                    Node* parent_node =
                        parent_stack->nodes[parent_stack_top - 1];
                    int parent_node_i = parent_indices[parent_stack_top - 1];
                    if (parent_node_i >= parent_node->nodes->capacity) {
                        increase_node_list_capacity(
                            parent_node->nodes,
                            parent_node->nodes->capacity + 5);
                    }
                    parent_node->nodes->nodes[parent_node_i] = focus_block;
                    parent_indices[parent_stack_top - 1]++;
                    parent_stack->nodes[parent_stack_top] = focus_block;
                    parent_indices[parent_stack_top] = focus_block_i;
                }
                current_node.type = BLOCK;
                current_node.nodes = initialize_node_list(5);
                focus_block_i = 0;
                if (held_header != NULL) {
                    current_node.nodes->nodes[0] = held_header;
                    held_header = NULL;
                    focus_block_i++;
                }
                focus_block = create_node();
                *focus_block = current_node;

                Node* parent_node = parent_stack->nodes[parent_stack_top];
                int parent_node_i = parent_indices[parent_stack_top];
                if (parent_node_i >= parent_node->nodes->capacity) {
                    increase_node_list_capacity(
                        parent_node->nodes, parent_node->nodes->capacity + 5);
                }
                parent_node->nodes->nodes[parent_node_i] = focus_block;
                parent_indices[parent_stack_top]++;
            }
            break;
        case RIGHT_BRACKET:
            (*i)++;
            if (focus_block == root_block) {
                free(parent_stack->nodes);
                free(parent_stack);
                return root_block;
            }
            focus_block = parent_stack->nodes[parent_stack_top];
            if (focus_block != root_block) {
                parent_stack->nodes[parent_stack_top] = NULL;
                parent_indices[parent_stack_top] = 0;
                parent_stack_top--;
            }
            break;
        case KEYWORD:
            if (!strncmp(tokens->tokens[*i]->value, "if", 2)) {
                held_header = parse_control_flow_header(tokens, i);
            } else {
                if (focus_block == root_block) {
                    if (parent_indices[0] >= root_block->nodes->capacity) {
                        increase_node_list_capacity(
                            root_block->nodes, root_block->nodes->capacity + 5);
                    }
                    root_block->nodes->nodes[parent_indices[0]] =
                        parse_statement(tokens, i);
                    parent_indices[0]++;
                } else {
                    if (focus_block_i >= focus_block->nodes->capacity) {
                        increase_node_list_capacity(
                            focus_block->nodes,
                            focus_block->nodes->capacity + 5);
                    }
                    focus_block->nodes->nodes[focus_block_i] =
                        parse_statement(tokens, i);
                    focus_block_i++;
                }
            }
            break;
        default:
            if (focus_block == root_block) {
                if (parent_indices[0] >= root_block->nodes->capacity) {
                    increase_node_list_capacity(
                        root_block->nodes, root_block->nodes->capacity + 5);
                }
                root_block->nodes->nodes[parent_indices[0]] =
                    parse_expression(tokens, i);
                parent_indices[0]++;
            } else {
                if (focus_block_i >= focus_block->nodes->capacity) {
                    increase_node_list_capacity(
                        focus_block->nodes, focus_block->nodes->capacity + 5);
                }
                focus_block->nodes->nodes[focus_block_i] =
                    parse_expression(tokens, i);
                focus_block_i++;
            }
            (*i)++;
            break;
        }
    }

    return NULL;
}

Node* parse_control_flow_header(TokenList* tokens, int* i) {
    Node* header = create_node();
    switch (tokens->tokens[*i]->type) {
    case KEYWORD:
        if (!strncmp(tokens->tokens[*i]->value, "if", 2)) {
            header->type = IF_H;
            (*i)++;
            header->rhs = parse_expression(tokens, i);
        }
        break;
    default:
        break;
    }

    return header;
}

Node* parse_function_header(TokenList* tokens, int* i) {
    Node* header = create_node();
    switch (tokens->tokens[*i]->type) {
    case KEYWORD:
        if (!strncmp(tokens->tokens[*i]->value, "int", 3)) {
            header->datatype = parse_datatype(tokens, i);
            header->name = calloc(50, 1);
            strncpy(header->name, tokens->tokens[*i]->value, 50);
            *i += 2;
            if (tokens->tokens[*i]->type != RIGHT_PARENTHESIS) {
                header->nodes = initialize_node_list(5);
                int node_index = 0;
                while (tokens->tokens[*i]->type != RIGHT_PARENTHESIS) {
                    if (node_index >= header->nodes->capacity) {
                        increase_node_list_capacity(
                            header->nodes, header->nodes->capacity + 5);
                    }
                    header->nodes->nodes[node_index] =
                        parse_statement(tokens, i);
                    node_index++;
                }
            }
            (*i)++;
            if (tokens->tokens[*i]->type == SEMICOLON) {
                header->type = FUNCTION_DECLARATION_H;
                (*i)++;
            } else {
                header->type = FUNCTION_DEFINITION_H;
            }
        }
        break;
    default:
        break;
    }

    return header;
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
    for (; tokens->tokens[*i]->type != IDENTIFIER; (*i)++)
        ;
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

// FIXME: freaking function call causes a memory leak.
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
                    expression->nodes = initialize_node_list(10);
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
            if (focus_node->type == FUNCTION_CALL_E &&
                tokens->tokens[*i + 1]->type != RIGHT_PARENTHESIS) {
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
                for (int node_index = 0;
                     node_index < left_paren_return_stack[return_stack_top - 1]
                                      ->nodes->capacity;
                     node_index++) {
                    if (left_paren_return_stack[return_stack_top - 1]
                            ->nodes->nodes[node_index] == NULL) {
                        left_paren_return_stack[return_stack_top - 1]
                            ->nodes->nodes[node_index] = create_node();
                        left_paren_return_stack[return_stack_top - 1]
                            ->nodes->nodes[node_index]
                            ->type = ARGUMENT_E;
                        left_paren_return_stack[return_stack_top - 1]
                            ->nodes->nodes[node_index]
                            ->precedence = 16;
                        focus_node =
                            left_paren_return_stack[return_stack_top - 1]
                                ->nodes->nodes[node_index];
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
    }
    if (node->type == VARIABLE_DECLARATION_S) {
        destroy_datatype(node->datatype);
        free(node->name);
    } else if (node->type == BLOCK) {
        destroy_node_list(node->nodes);
        free(node);
        return;
    } else if ((node->type == FUNCTION_DEFINITION_H) ||
               (node->type == FUNCTION_DECLARATION_H)) {
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
    void* test = realloc((void*)(array->nodes), new_capacity * sizeof(Node*));
    fg_bright_red();
    if (test == NULL) {
        printf("Realloc failed.\n");
        return;
    }
    array->nodes = (Node**)test;
    clear();
    for (int i = array->capacity; i < new_capacity; i++) {
        array->nodes[i] = NULL;
    }
    array->capacity = new_capacity;
}
