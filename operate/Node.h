//
// Created by joshu on 2022/5/23 0023.
//

#ifndef GITLAB_NODE_H
#define GITLAB_NODE_H

#include "defines.h"

#include <cstdint>
#include <cmath>
#include <limits>

#include <cassert>
#ifdef DEBUG
#include <cstdio>
#endif

class Node {
public:
    Node *parent;
    Node *children[12];
    uint64_t win_c;
    uint64_t visit_c;
    uint8_t child_c;
    uint8_t operation;
    Situation finalized;
    Player side;

    inline void init() {
        parent = nullptr;
        win_c = 0;
        visit_c = 0;
        child_c = 0;
        operation = 0xff;
        finalized = Situation::None;
        side = Player::None;
    }

    [[nodiscard]] inline bool expandable () const {
        return finalized == Situation::None;
    }

    [[nodiscard]] inline Situation get_finalized_result() const  {
        return finalized;
    }

    [[nodiscard]] inline double value() const {
        if (visit_c == 0) {
            return std::numeric_limits<double>::max();
        } else {
            return static_cast<double>(win_c) / visit_c + UCB_C * sqrt(2 * log(parent->visit_c) / visit_c);
        }
    }

    [[nodiscard]] inline uint8_t children_count() {
        return child_c;
    }

    [[nodiscard]] inline uint8_t get_operation() {
        assert (parent && operation != 0xff);
        return operation;
    }

    [[nodiscard]] Node *max_child() const ;

#ifndef DECISION_INFO
    [[nodiscard]]
#endif
    Node *most_visited_child() const ;

    [[nodiscard]] inline Node *operator [] (uint8_t idx) {
        assert(idx < child_c);
        return children[idx];
    }

    [[nodiscard]] inline uint64_t visit_count () const {
        return visit_c;
    }

    [[nodiscard]] inline Node *get_parent() const {
        return parent;
    }

    void expand(uint8_t *positions, uint8_t count);

    inline void reverse_side(Node *ch) {
        if (side == Player::Self) {
            ch->side = Player::Other;
        } else  {
            ch->side = Player::Self;
        }
    }

    inline void update(Situation res) {
        ++visit_c;
        if (res == Situation::SelfWin && side == Player::Self) {
            ++win_c;
        } else if (res == Situation::OtherWin && side == Player::Other) {
            ++win_c;
        }
    }

    inline void set_final_result(Situation s) {
        assert(s != Situation::None);
        finalized = s;
    }

    inline void set_side(Player _side) {
        side = _side;
    }

    Node *match_child(uint8_t y);
};


const size_t NODE_SPACE_CNT = NODE_SPACE / sizeof(Node);
extern uint8_t *pool[NODE_SPACE_CNT * sizeof(Node)];
extern uint64_t node_pool_ptr;
Node *alloc();
void reset_pool();
bool inherit_tree();

#endif //GITLAB_NODE_H
