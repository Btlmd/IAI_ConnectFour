//
// Created by joshu on 2022/5/23 0023.
//

#ifndef GITLAB_GAME_H
#define GITLAB_GAME_H


#include "defines.h"
#include "Node.h"
#include "../Point.h"

#include <cstdint>
#include <cstdlib>

#ifdef DEBUG
#include <cstdio>
#endif
#include <cstdio>

class Game {
public:

    Game(int _M, int _N, int **_board, const int *_top, int noX, int noY, int _lastY);

    ~Game() {
        delete ckpt;
    };

    Situation eval() ;

    void step(uint8_t y);

    inline void checkpoint() {
        delete ckpt;
        ckpt = new Game {*this};
    }

    inline void restore() {
        restore_from(*ckpt);
    }

    uint8_t gen_available_positions(uint8_t *&);

    Situation simulate();

    inline void switch_player() {
        if (player == Player::Self) {
            player = Player::Other;
        } else {
            player = Player::Self;
        }
    }

    template<uint8_t n>
    bool situation_win() const;

    bool situation_tie();

    inline Point column_to_operation(uint8_t y) {
        return {top[y] - 1, y};
    }

    bool favorable_move();

    inline bool last_finalized() {
        return last_sim_finalized;
    }

    void show_tops() const {

        fprintf(stderr, "TOP [");
        for (int i {0}; i < N; ++i) {
            fprintf(stderr, "%d ", top[i]);
        }
        fprintf(stderr, "]\n");

    }

    void show_board() const {
        fprintf(stderr, "BOARD\n");
        for (int i {0}; i < M; ++i) {
            for (int j {0}; j < N; ++j) {
                if (board[i][j] == 0)
                    fprintf(stderr, ". ");
                else
                    fprintf(stderr, "%d ", board[i][j]);
            }
            fprintf(stderr, "\n");
        }

    }

    void repr() const {
        char format1[] = "const int M_ {%d}, N_ {%d}, noX_{%d}, noY_{%d};"
                         "int dbg_board[M_][N_]={";
        fprintf(stderr, format1, M, N, noX, noY);
        for (int i {0}; i < M; ++i) {
            fprintf(stderr, "{");
            for (int j {0}; j < N; ++j) {
                fprintf(stderr, "%d, ", board[i][j]);
            }
            fprintf(stderr, "},");

        }
        char format2[] = "};\n"
                         "for (int i = 0; i < M_; i++)"
                         "{board[i] = new int[N_];for (int j = 0; j < N_; j++){board[i][j] = ((int*)dbg_board)[i * N_ + j];}}"
                         "int dbg_top[N_] = {";
        fprintf(stderr, format2);
        for (int i {0}; i < N; ++i) {
            fprintf(stderr, "%d, ", top[i]);
        }
        char format3[] =  "};\n"
                          "Game game{M_, N_, board, dbg_top, noX_, noY_, %d};\n";
        fprintf(stderr, format3, lastY);
    }


    inline void set_expansion_callback(Node *n) {
        expansion_callback = n;
    }

    inline void clear_expansion_callback() {
        expansion_callback = nullptr;
    }

    inline uint8_t get_last_operation() const  {
        return lastY;
    }


private:
    inline static int rng(int upper) {
        return rand() % upper;
    }

    void restore_from(const Game &g);

    uint8_t refresh_available_positions();

    class Tentative {
    public:
        Tentative (Game *_g): g{_g} {}
        void step(uint8_t y);
        void unstep();
        void undo();
        void switch_player();
        void unswitch();
        void store_av();
        void restore_av();

    private:
        uint8_t last_tentative_Y {0xff};
        uint8_t previous_last_Y {0xff};
        bool switched {false};
        bool av_stored {false};
        uint8_t available[N_MAX];
        Game *g {nullptr};
    };

    uint8_t board[M_MAX][N_MAX];
    uint8_t top[N_MAX];
    uint8_t available[N_MAX];
    uint8_t favorable[N_MAX];
    uint8_t M, N, available_cnt, favorable_cnt, noX, noY;
    uint8_t lastY{0xff};
    bool last_sim_finalized {false};
    Player player {Player::Other};
    Game *ckpt {nullptr};
    Node *expansion_callback {nullptr};
};

#endif //GITLAB_GAME_H
