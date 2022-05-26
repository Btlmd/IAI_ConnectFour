//
// Created by joshu on 2022/5/23 0023.
//

#ifndef GITLAB_DEFINES_H
#define GITLAB_DEFINES_H

const int M_MAX {12};
const int N_MAX {12};
const double UCB_C {0.9};
const double DURATION_LIMIT {2.65};

const unsigned char NONE {0};
const unsigned char SELF {2};
const unsigned char OTHER {1};
const unsigned char BLOCK {3};

const unsigned long long NODE_SPACE {800000000};
const unsigned long long MARGIN_NODE_CNT {1000000};

enum class Player {
    None = NONE,
    Self = SELF,
    Other = OTHER
};

enum class Situation {
    SelfWin = SELF,
    OtherWin = OTHER,
    Tie,
    None
};

//#define DEBUG

//#define VEROBSE

#define DECISION_INFO

#ifndef DEBUG
#define NDEBUG
#endif

#endif //GITLAB_DEFINES_H
