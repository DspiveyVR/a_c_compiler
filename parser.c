#include <stdlib.h>
#include <stdint.h>
#include "lexer.h"

typedef enum NodeType {
    NONETYPE,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
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
    // parenthetical expression has been fully parsed if left_paren_return
    // is not NULL.  Then left_paren_return is reassigned to NULL.
    Node* left_paren_return = NULL;
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
                current_node.type = MULTIPLICATION;
                current_node.precedence = 3;
                if ((current_node.precedence > focus_node->precedence) ||
                        (focus_node->type == INTEGER)) {
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
                    expression->type = OPEN_PARENTHESIS;
                    expression->precedence = 0;
                    left_paren_return = focus_node;
                    continue;
                }
                current_node.type = OPEN_PARENTHESIS;
                current_node.precedence = 0;
                focus_node->rhs = create_node();
                *(focus_node->rhs) = current_node;
                focus_node = focus_node->rhs;
                left_paren_return = focus_node;
                break;
            case RIGHT_PARENTHESIS:
                current_node.type = CLOSE_PARENTHESIS;
                current_node.precedence = 0;
                focus_node->rhs->rhs = create_node();
                *(focus_node->rhs->rhs) = current_node;
                focus_node = left_paren_return;
                break;
            case NONE:
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
    return node;
}

void destroy_node(Node* node) {
    if (node->lhs != NULL) {
        destroy_node(node->lhs);
    }
    if (node->rhs != NULL) {
        destroy_node(node->rhs);
    }
    free(node);
}
