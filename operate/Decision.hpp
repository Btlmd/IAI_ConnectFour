#ifndef DECISION
#define DECISION

#include "defines.h"
#include "Node.h"
#include "Game.h"

#include "../Point.h"
#include <chrono>
#include <stack>

#ifdef DEBUG

#endif
#include <cstdio>

class Decision {
public:

    Decision(Game *_game): game {_game} {}

    void backward(Node *leaf, Situation res) {
        assert(res != Situation::None);
        auto ptr {leaf};
        while (ptr) {
            ptr->update(res);
            if (ptr == root) {
                break;
            }
            ptr = ptr->get_parent();
        }
    }


    static void reset_root() {
#ifdef DECISION_INFO
        fprintf(stderr, "Tree re-constructed\n");
#endif
        reset_pool();
        root = alloc();
        root->set_side(Player::Other);
    }

    Point operator()() {
#ifdef DECISION_INFO
        fprintf(stderr, "\n\nDecision \n");
        game->repr();
        game->show_board();
#endif
//        assert(false);


        game->checkpoint();

        Node *new_root {nullptr};

        if (root && inherit_tree()) {
#ifdef DECISION_INFO
            fprintf(stderr, "Try to inherit tree\n");
#endif
            new_root = root->match_child(game->get_last_operation());
        }
#ifdef DECISION_INFO
        else {
            fprintf(stderr, "Corresponding node missing\n");
        }
#endif

        if (new_root) {
            root = new_root;
        } else {
            reset_root();
        }

#ifdef DECISION_INFO
        root->most_visited_child();
#endif

        assert(root->side == Player::Other);

        auto begin = std::chrono::steady_clock::now();

#ifdef DECISION_INFO
        uint64_t this_time_visit_count {0};
#endif

        while (true) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - begin
            ).count();
            if (delta / 1000.0 > DURATION_LIMIT)
                break;

            auto ptr {root};

            // select
            while (ptr->children_count()) {
                ptr = ptr->max_child();
                game->step(ptr->get_operation());
            }

            // deprecated expand
            if (ptr->visit_count() && ptr->expandable() && !ptr->children_count()) {
                uint8_t *positions;
                auto pos_cnt {game->gen_available_positions(positions)};
                if (pos_cnt) {
                    ptr->expand(positions, pos_cnt);
                    ptr = (*ptr)[0];
                    game->step(ptr->get_operation());
                }
            }

            // simulate
            Situation res;

            if (!ptr->expandable()) {
                res = ptr->get_finalized_result();
            } else {
                game->set_expansion_callback(ptr);
                res = game->simulate();
                game->clear_expansion_callback();

                if (game->last_finalized()) {
                    ptr->set_final_result(res);
                }
            }

            // backward
            backward(ptr, res);

            // restore the initial game setting
            game->restore();
#ifdef DECISION_INFO
            ++this_time_visit_count;
#endif
        }

#ifdef DECISION_INFO
        fprintf(stderr, " > Visit This Round: %ld\n", this_time_visit_count);
#endif

        auto mvc = root->most_visited_child();

        uint8_t column_choice {mvc->get_operation()};
#ifdef DECISION_INFO
        fprintf(stderr, "Total simulation: %ld\nOperation @ %d\nNode usage: %ld\n", root->visit_c, column_choice, node_pool_ptr);
#endif
        mvc->most_visited_child();
        root = mvc;
        return game->column_to_operation(column_choice);
    }

private:
    Game *game {nullptr};
    static Node *root;
};

Node *Decision::root {nullptr};

#endif
