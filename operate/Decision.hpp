#ifndef DECISION
#define DECISION

#include "defines.h"
#include "Node.h"
#include "Game.h"

#include "../Point.h"
#include <chrono>
#include <stack>

#ifdef DEBUG
#include <cstdio>
#endif


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
#ifdef DEBUG
        fprintf(stderr, "\nDecision \n");
        game->show_tops();
        game->show_board();
#endif
        auto begin = std::chrono::steady_clock::now();

        game->checkpoint();
        auto root {Node::gen_root()};

        while (true) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - begin
            ).count();
            if (delta / 1000.0 > DURATION_LIMIT)
                break;

            // initialize game and search settings
            path_top = 0;
            auto ptr {&root};

            // select
            while (ptr->children_count()) {
                path[path_top++] = ptr;
                ptr = ptr->max_child();
                game->step(ptr->get_operation());
            }

            // expand
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
            auto res = game->simulate();
            if (game->last_finalized()) {
                ptr->set_final_result(res);
            }


            // backward
            backward(ptr, res);

            // restore the initial game setting
            game->restore();
        }

        uint8_t column_choice {root.most_visited_child()->get_operation()};
#ifdef DEBUG
        fprintf(stderr, "Operation @ %d\n", column_choice);
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
