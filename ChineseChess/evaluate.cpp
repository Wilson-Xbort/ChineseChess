#include "evaluate.h"
#include "arr_board.h"


// struct Chess_Pieces_Value chess_pv;
#define ADVANCED_VALUE 3

int evaluate()
{
    return (chess_pos.self_player == 0 ? chess_pv.White_Value - chess_pv.Black_Value
        : chess_pv.Black_Value - chess_pv.White_Value) + ADVANCED_VALUE;
}
