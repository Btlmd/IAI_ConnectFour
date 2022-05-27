//
// Created by joshu on 2022/5/23 0023.
//

#include "Node.h"

#include <cstdio>

uint8_t *pool[NODE_SPACE_CNT * sizeof(Node)];
uint64_t node_pool_ptr {1};
Node *alloc() {
    node_pool_ptr %= NODE_SPACE_CNT;

#ifdef POOL_INFO
    if (node_pool_ptr == 0) {
        fprintf(stderr, "<Error: Pool Reset to 0>\n");
    }
#endif

    auto p {(Node *) pool};
    p[node_pool_ptr].init();
    return &p[node_pool_ptr++];
}

bool inherit_tree() {
    return node_pool_ptr + MARGIN_NODE_CNT < NODE_SPACE_CNT;
}

void reset_pool() {
#ifdef POOL_INFO
    fprintf(stderr, "<Info: Pool Reset to 1>\n");
#endif
    node_pool_ptr = 1;
}

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
#ifdef DECISION_INFO
    fprintf(stderr, "\n[NODE INFO]\nVisits: %ld\nChoices: ", visit_c);
#endif
    for (uint8_t i {0}; i < child_c; ++i) {
#ifdef DECISION_INFO
        fprintf(stderr, "(%d, %ld, %ld)", children[i]->operation, children[i]->win_c, children[i]->visit_c);
#endif
        if (children[i]->visit_c > max_visit_count) {
            max_visit_count = children[i]->visit_c;
            max_visit_at = children[i];
        }
    }
//    assert(max_visit_at);
#ifdef DECISION_INFO
    fprintf(stderr, "\n");
#endif
    return max_visit_at;
}

void Node::expand(uint8_t *positions, uint8_t count) {
    assert(child_c == 0);
    assert(count > 0);
    for (int i {0}; i < count; ++i) {
        children[i] = alloc();
        children[i]->operation = positions[i];
        children[i]->parent = this;
        reverse_side(children[i]);
    }
    child_c = count;
}

Node *Node::match_child(uint8_t y) {
    for (int i {0}; i < child_c; ++i) {
        if (children[i]->get_operation() == y) {
            return children[i];
        }
    }
    return nullptr;
}
