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
            ptr = ptr->get_parent();
        }
    }


    Point operator()() {
#ifdef DECISION_INFO
        fprintf(stderr, "\n\nDecision \n");
        game->repr();
        game->show_board();
#endif
        auto begin = std::chrono::steady_clock::now();

        game->checkpoint();
        reset_pool();
        auto root = alloc();
        root->set_side(Player::Other);

        while (true) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - begin
            ).count();
            if (delta / 1000.0 > DURATION_LIMIT)
                break;

            // initialize game and search settings
            path_top = 0;
            auto ptr {root};

            // select
            while (ptr->children_count()) {
                path[path_top++] = ptr;
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
        }

        uint8_t column_choice {root->most_visited_child()->get_operation()};
#ifdef DECISION_INFO
        fprintf(stderr, "Total simulation: %ld\nOperation @ %d\nNode usage: %ld\nNode Size: %d\n", root->visit_c, column_choice, node_pool_ptr, sizeof(Node));
#endif
        return game->column_to_operation(column_choice);
    }

private:
    uint8_t M, N;
    Node *path[MAX_PATH];
    size_t path_top {0};
    Game *game {nullptr};
};

#endif
