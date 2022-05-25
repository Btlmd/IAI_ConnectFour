//
// Created by joshu on 2022/5/23 0023.
//

#include "Game.h"
#include <cstdlib>
#include <cstring>
#include "../Judge.h"

void Game::step(uint8_t y) {
    switch_player();
    top[y]--;

    if (board[top[y]][y] == BLOCK) {
        top[y]--;
    }

    assert(top[y] != 0xff);
    board[top[y]][y] = static_cast<uint8_t>(player);
    lastY = y;
    // here, the player denotes the player who had just played

#ifdef VERBOSE
    fprintf(stderr, "Game Step @ %d\n", y);
    show_tops();
    show_board();
    fprintf(stderr, "\n");
#endif
}

/*
 *  if true, Game object modified
 *  if false, it will take care of the undo of the Game
 */
bool Game::favorable_move() {
    auto pos_cnt {refresh_available_positions()};
    Tentative t {this};

#ifdef VERBOSE
    fprintf(stderr, "\nFav Move\n");
#endif

    for (int i {0}; i < pos_cnt; ++i) {
        t.switch_player();
        t.step(available[i]);

        auto res{eval()};

        // immediate win
        if (res == Situation::SelfWin) {
            return true;
        }
        t.undo();
    }

    uint8_t all_cnt {0};
    uint8_t possible_cnt {0};
    uint8_t deprecated_cnt {0};
    uint8_t possible_positions[N_MAX];
    uint8_t deprecated_positions[N_MAX];
    uint8_t all_positions[N_MAX];

    for (int i {0}; i < pos_cnt; ++i) {
        t.switch_player();
        t.step(available[i]);

        // search for the other player to be connect
        t.store_av();
        bool other_player_c4 {false};
        bool other_player_c3 {false};

        auto pos_after {refresh_available_positions()};

        for (int j {0}; j < pos_after; ++j) {
            Tentative opt {this};
            opt.switch_player();
            opt.step(available[j]);

            if (situation_win<4>()) {
                other_player_c4 = true;
                opt.undo();
                break;
            }

//            if (situation_win<3>()) {
//                other_player_c3 = true;
//            }


            opt.undo();
        }
        t.restore_av();
        t.undo();

        if (other_player_c4) {
            continue;
        }

//        if (other_player_c3) {
//            deprecated_positions[deprecated_cnt++] = available[i];
//        } else {
//            possible_positions[possible_cnt++] = available[i];
//        }
        all_positions[all_cnt++] = available[i];
    }

    if (expansion_callback) {
        if (all_cnt) {
            expansion_callback->expand(all_positions, all_cnt);
        }
        clear_expansion_callback();
    }

//    if (possible_cnt) {
//        auto choice = rng(possible_cnt);
//        step(possible_positions[choice]);
//        return true;
//    }
//
//    if (deprecated_cnt) {
//        auto choice = rng(deprecated_cnt);
//        step(deprecated_positions[choice]);
//        return true;
//    }

    if (all_cnt) {
        auto choice = rng(all_cnt);
        step(all_positions[choice]);
        return true;
    }

    return false;
}

Game::Game(int _M, int _N, int **_board, const int *_top, int _noX, int _noY, int _lastY) :
    M{static_cast<uint8_t>(_M)},
    N{static_cast<uint8_t>(_N)},
    noX {static_cast<uint8_t>(_noX)},
    noY {static_cast<uint8_t>(_noY)},
    lastY {static_cast<uint8_t>(_lastY)} {
    for (uint8_t i {0}; i < M; ++i) {
        for (uint8_t j {0}; j < N; ++j) {
            board[i][j] = static_cast<uint8_t>(_board[i][j]);
        }
    }
    for (uint8_t i {0}; i < N; ++i) {
        top[i] = static_cast<uint8_t>(_top[i]);
    }
    board[noX][noY] = BLOCK;
}

void Game::restore_from(const Game &g) {
    M = g.M;
    N = g.N;
    player = g.player;
    memcpy(top, g.top, N);
    memcpy(board, g.board, M_MAX * N_MAX);
}

uint8_t Game::refresh_available_positions() {
    uint8_t ptr {0};
    for (uint8_t i {0}; i < N; ++i) {
        if (top[i] && !(top[i] == 1 && board[0][i] == BLOCK)) {
            available[ptr++] = i;
        }
    }
    available_cnt = ptr;
    return ptr;
}

Situation Game::simulate() {
#ifdef VERBOSE
    fprintf(stderr, "Simulate\n");
#endif
    bool first_flag {true};
    last_sim_finalized = false;

    while (true) {
        auto situation = eval();

        if (situation != Situation::None) {
            if (first_flag) {
                last_sim_finalized = true;
            }

            return situation;
        } else {
            if (!favorable_move()) {
#ifdef VERBOSE
                fprintf(stderr, "Unfavorable\n");
#endif
                refresh_available_positions();
                auto choice = rng(available_cnt);
                step(available[choice]);
            }
        }

        first_flag = false;
    }
}

uint8_t Game::gen_available_positions(uint8_t *&positions) {
    positions = available;
    return refresh_available_positions();
}

Situation Game::eval()  {
    if (situation_win<4>()) {
        return player == Player::Self ? Situation::SelfWin : Situation::OtherWin;
    } else if (situation_tie()) {
        return Situation::Tie;
    }
    return Situation::None;
}

bool Game::situation_tie() {
    return !refresh_available_positions();
}

template <uint8_t n>
bool Game::situation_win() const {
    int player_code = static_cast<int>(player);
    int y {lastY}, x {top[lastY]};

    //横向检测
    int i, j;
    int count = 0;
    for (i = y; i >= 0; i--)
        if (!(board[x][i] == player_code))
            break;
    count += (y - i);
    for (i = y; i < N; i++)
        if (!(board[x][i] == player_code))
            break;
    count += (i - y - 1);
    if (count >= n)
        return true;

    //纵向检测
    count = 0;
    for (i = x; i < M; i++)
        if (!(board[i][y] == player_code))
            break;
    count += (i - x);
    if (count >= n)
        return true;

    //左下-右上
    count = 0;
    for (i = x, j = y; i < M && j >= 0; i++, j--)
        if (!(board[i][j] == player_code))
            break;
    count += (y - j);
    for (i = x, j = y; i >= 0 && j < N; i--, j++)
        if (!(board[i][j] == player_code))
            break;
    count += (j - y - 1);
    if (count >= n)
        return true;

    //左上-右下
    count = 0;
    for (i = x, j = y; i >= 0 && j >= 0; i--, j--)
        if (!(board[i][j] == player_code))
            break;
    count += (y - j);
    for (i = x, j = y; i < M && j < N; i++, j++)
        if (!(board[i][j] == player_code))
            break;
    count += (j - y - 1);
    if (count >= n)
        return true;

    return false;
}

void Game::Tentative::step(uint8_t y) {
    assert(last_tentative_Y == 0xff);
    last_tentative_Y = y;
    previous_last_Y = g->lastY;

    g->top[y]--;
    assert(g->top[y] != 0xff);
    g->board[g->top[y]][y] = static_cast<uint8_t>(g->player);
    g->lastY = last_tentative_Y;

#ifdef VERBOSE
    fprintf(stderr, "Tentative Step @ %d\n", y);
    g->show_tops();
    g->show_board();
    fprintf(stderr, "\n");
#endif
}


void Game::Tentative::undo() {
    assert(last_tentative_Y != 0xff);

    // restore board
    g->board[g->top[last_tentative_Y]][last_tentative_Y] = NONE;

    // restore top
    g->top[last_tentative_Y]++;
    assert(g->top[last_tentative_Y] <= g->M);

    // restore lastY
    g->lastY = previous_last_Y;

#ifdef VERBOSE
    fprintf(stderr, "Tentative Undo @ %d\n", last_tentative_Y);
    g->show_tops();
    g->show_board();
    fprintf(stderr, "\n");
#endif

    last_tentative_Y = 0xff;

    if (switched) {
        g->switch_player();
        switched = false;
    }

}

void Game::Tentative::switch_player() {
    assert(!switched);
    switched = true;
    g->switch_player();
}

void Game::Tentative::store_av() {
    assert (!av_stored);
    av_stored = true;
    memcpy(available, g->available, N_MAX);
}

void Game::Tentative::restore_av() {
    assert(av_stored);
    av_stored = false;
    memcpy(g->available, available, N_MAX);
}

void Game::Tentative::unstep() {

}

void Game::Tentative::unswitch() {

}
