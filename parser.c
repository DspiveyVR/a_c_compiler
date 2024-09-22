#include <stdlib.h>
#include <stdint.h>
#include "lexer.h"
#include <stdio.h>

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


Node* parse_expression(TokenList* tokens) {
    Node* expression = create_node();
    Node* focus_node = expression;

    for (int i = 0; i < tokens->length; i++) { 
        Node current_node;
        current_node.lhs = NULL;
        current_node.rhs = NULL;

        switch (tokens->tokens[i]->type) {
            case CONSTANT:
                if (expression->type == NONETYPE) {
                    expression->type = INTEGER;
                    expression->value = atoi(tokens->tokens[i]->value);
                    expression->precedence = 16;
                    continue;
                }
                current_node.type = INTEGER;
                current_node.value = atoi(tokens->tokens[i]->value);
                current_node.precedence = 16;
                // Integers always have the lowest precedence.
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                break;
            case PLUS:
                current_node.type = ADDITION;
                current_node.precedence = 4;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
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
                current_node.type = MULTIPLICATION;
                current_node.precedence = 3;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER)) {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *focus_node;
                    *focus_node = current_node;
                } else {
                    current_node.lhs = create_node();
                    *(current_node.lhs) = *(focus_node->rhs);
                    *(focus_node->rhs) = current_node;
                    focus_node = focus_node->rhs;
                }
                break;
            case NONE:
                return expression;
        }
    }
    return expression;
} 

Node* create_node() {
    Node* node = (Node*)malloc(sizeof(Node));
    node->lhs = NULL;
    node->rhs = NULL;
    return node;
}

void destroy_node(Node* node) {
    //bals
}
