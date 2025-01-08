#ifndef GAME_H
#define GAME_H

typedef enum move {
    ROCK,
    PAPER,
    SCISSORS,
}move_t;

typedef enum winner {
    LOSE,
    WIN,
    TIE,
    INVALID_INPUT
}winner_t;

winner_t rps (move_t my_move, move_t op_move);

#endif