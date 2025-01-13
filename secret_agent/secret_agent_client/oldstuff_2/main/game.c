#include "printer_helper.h"
#include "game.h"

winner_t rps(move_t my_move, move_t op_move) 
{
    PRINTFC_GAME("RPS starting");
    switch(my_move) 
    {
        case ROCK:
            switch (op_move) 
            {
                case ROCK:
                    return TIE;
                case PAPER:
                    return LOSE;
                case SCISSORS:
                    return WIN;
                default:
                    return INVALID_INPUT;
            }
        case PAPER:
            switch (op_move) 
            {
                case ROCK:
                    return WIN;
                case PAPER:
                    return TIE;
                case SCISSORS:
                    return LOSE;
                default:
                    return INVALID_INPUT;
            }
        case SCISSORS:
            switch (op_move) 
            {
                case ROCK:
                    return LOSE;
                case PAPER:
                    return WIN;
                case SCISSORS:
                    return TIE;
                default:
                    return INVALID_INPUT;
            }
        default:
            return INVALID_INPUT;
    }
}