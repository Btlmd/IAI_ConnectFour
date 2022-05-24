//
// Created by joshu on 2022/5/23 0023.
//

#include "Node.h"
#include <cassert>


Node *Node::max_child() const {
    assert(child_c);
    Node *max_at {nullptr};
    double max_val {-1};
    for (int i {0}; i < child_c; ++i) {
        auto val {children[i]->value()};
        if (val > max_val) {
            max_val = val;
            max_at = children[i];
        }
    }
    return max_at;
}

Node *Node::most_visited_child() const {
    uint64_t max_visit_count {0};
    Node *max_visit_at {nullptr};
#ifdef DEBUG
    fprintf(stderr, "Choices: ");
#endif
    for (uint8_t i {0}; i < child_c; ++i) {
#ifdef DEBUG
        fprintf(stderr, "(%d, %ld, %ld)", children[i]->operation, children[i]->win_c, children[i]->visit_c);
#endif
        if (children[i]->visit_c > max_visit_count) {
            max_visit_count = children[i]->visit_c;
            max_visit_at = children[i];
        }
    }
    assert(max_visit_at);
#ifdef DEBUG
    fprintf(stderr, "\n");
#endif
    return max_visit_at;
}

void Node::expand(uint8_t *positions, uint8_t count) {
    assert(count > 0);
    for (int i {0}; i < count; ++i) {
        children[i] = new Node;
        children[i]->operation = positions[i];
        children[i]->parent = this;
        reverse_side(children[i]);
    }
    child_c = count;
}
