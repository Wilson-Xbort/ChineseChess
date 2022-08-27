#include "crule.h"
#include "arr_board.h"
#include "gui.h"
#include "search.h"
#include <math.h>

using namespace search;
struct Movegen* mov = (struct Movegen*)malloc(sizeof(Movegen));
struct MovePieces mp;
static int tmp_king_pos;

int MovePieces::MovePiece(int mv) {
    int start = start_map_arr(mv);
    int end = end_map_arr(mv);
    int mv_piece = chess_pos.ucpc_Squares[start];
    int destination = chess_pos.ucpc_Squares[end];
    if (destination != 0) {
        if (destination < 8)
            chess_pv.White_Value -= chess_pieces_valtab[destination - 1][end];
        else
            chess_pv.Black_Value -= chess_pieces_valtab[destination - 8][overturn_result(end)];
    }
    chess_pos.ucpc_Squares[end] = chess_pos.ucpc_Squares[start];
    chess_pos.ucpc_Squares[start] = 0;
    return destination;
}

void MovePieces::undo_move_pieces(int mv, int tp_pieces) {
    int start = start_map_arr(mv);
    int end = end_map_arr(mv);
    int destination = chess_pos.ucpc_Squares[end];
    chess_pos.ucpc_Squares[start] = destination;
    chess_pos.ucpc_Squares[end] = tp_pieces;
    // exchange_player();
}

void MovePieces::undo_move_orgn(int mv, int tp_pieces) {
    search_process.original_distance--;
    exchange_player();
    undo_move_pieces(mv, tp_pieces);
}

int MovePieces::move_pieces(int mv, int& tp_pieces) {
    int king_val = chess_pos.self_player * 7 + 1;

    tp_pieces = MovePiece(mv); // 移动到目标所在的棋子类型
    if (mov->is_checked(king_val)) {
        undo_move_pieces(mv, tp_pieces);
        return false;
    }
    exchange_player();
    search_process.original_distance++;
    return true;
}

static int grid_calculate(int king_pos, int player) {
    return king_pos - 16 + (player << 5);
}

bool Movegen::move_is_legal(int move) const  // const pointer
{
    int start, end;
    int starting_point, ending_point;
    int tag = side_tag(chess_pos.self_player);

    // judgement starting point has own pieces?
    start = start_map_arr(move);
    starting_point = chess_pos.ucpc_Squares[start];
    if (((starting_point == 0 ? 0 : starting_point + 8) & tag) == 0)
        goto failed;
   
    // judgement destination has own pieces?
    end = end_map_arr(move);
    ending_point = chess_pos.ucpc_Squares[end];
    if (((ending_point == 0 ? 0 : ending_point + 8) & tag) != 0)
        goto failed;

    switch (starting_point - RP > 0 ? starting_point - RP : starting_point) {
        case RK:
            return is_in_palance(end) && isking_step_legal(start, end); break;
        case RA:
            return is_in_palance(end) && isadvisor_step_legal(start, end); break;
        case RB:
            return is_in_board(end) && is_same_side(start, end) && isbishop_step_legal(start, end) && isbishop_eye_empty(start, end); break;
        case RN:
            return is_in_board(end) && isknight_step_legal(start, end); break;
        case RR:
            return is_in_board(end) && is_straight_line(start, end); break;
        case RC:
            return is_in_board(end) && is_cannon_step_legal(start, end); break;
        case RP:
            return is_in_board(end) && is_pawn_step_legal(start, end); break;
    }

 failed:
    return false;
}

// 王的步法是否合法
bool Movegen::isking_step_legal(int start, int end) const
{
    return ((int)((abs(end - start)) >> 4) + (abs(end - start) % 16)) <= king_step;
}

// 士的步法是否合法
bool Movegen::isadvisor_step_legal(int start, int end) const
{
    int start_row = (int)(start >> 4); // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);     //  (start >> 4) >> 0;   


    int start_col = start % 16;
    int end_col = end % 16;
    int steps = abs(start_row - end_row) + abs(start_col - end_col);

    if (start_row == end_row || start_col == end_col) return false;
    return steps == advisor_step;
}

bool Movegen::isbishop_step_legal(int start, int end) const
{
    int start_row = (int)(start >> 4); // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);     //  (start >> 4) >> 0;   

    int start_col = start % 16;
    int end_col = end % 16;

    if (abs(start_row - end_row) != 2 || abs(start_col - end_col) != 2)
        return false;

    if (start_row == end_row || start_col == end_col) return false;
    int steps = abs(start_row - end_row) + abs(start_col - end_col);
    return steps == bishop_step;
}

bool Movegen::isknight_step_legal(int start, int end) const
{
    knight_vector direction(none_k);
    int start_row = (int)(start >> 4);  // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);      //  (start >> 4) >> 0;   

    int start_col = start % 16;
    int end_col = end % 16;

    if (abs(start_row - end_row) > abs(start_col - end_col)) {  // 垂直
        if ((start_row - end_row) > 0) {  // top
            direction = (start_col - end_col > 0) ? top_left : top_right;
        }
        else if ((start_row - end_row) < 0) { // bottom
            direction = (start_col - end_col > 0) ? bottom_left : bottom_right;
        }
    }
    else if (abs(start_row - end_row) < abs(start_col - end_col)) { // 水平
        if ((start_col - end_col) > 0) {  // left
            direction = (start_row - end_row > 0) ? left_top : left_bottom;
        }
        else if ((start_col - end_col) < 0) { // right
            direction = (start_row - end_row > 0) ? right_top : right_bottom;
        }
    }

    if (direction == none_k) return false;

    switch (direction)
    {
    case top_left:
    case top_right:
        if (chess_pos.ucpc_Squares[start - ROW_COUNT] != 0)
            goto failed;
            break;
    case bottom_left:
    case bottom_right:
        if (chess_pos.ucpc_Squares[start + ROW_COUNT] != 0)
            goto failed;
        break;
    case left_top:
    case left_bottom:
        if (chess_pos.ucpc_Squares[start - 1] != 0)
            goto failed;
            break;
    case right_top:
    case right_bottom:
        if (chess_pos.ucpc_Squares[start + 1] != 0)
            goto failed;
            break;
    default:
        goto failed;
    }
    
    if (start_row == end_row || start_col == end_col) goto failed;

    return abs(start_row - end_row) + abs(start_col - end_col) == knight_step;

failed:
    return false;
}

bool Movegen::is_cannon_step_legal(int start, int end) const
{
    int start_grid_x = grid_y(start) - 3;
    int end_grid_x = grid_y(end) - 3;
    int start_grid_y = grid_x(start) - 3;
    int end_grid_y = grid_x(end) - 3;

    if (abs(start_grid_x - end_grid_x) > 0 && (start_grid_y - end_grid_y == 0)) {  // 平移
        return cannon_determine(start, end, 1);
    }

    if ((start_grid_x - end_grid_x == 0) && (abs(start_grid_y - end_grid_y) > 0)) {  // 垂直
        return cannon_determine(start, end, 2);
    }

    return false;
}

bool Movegen::is_pawn_step_legal(int start, int end) const
{
    if (u_overturn == true) {
        if (is_across_the_river(start, chess_pos.self_player) == true) { // 未过河
            return check_general_pawn_move(start, end);
        }
        else {
            return check_acrossed_pawn_move(start, end);
        }
    }
    else {
        if (ois_across_the_river(start, chess_pos.self_player) == true) { // 未过河
            return check_general_pawn_move(start, end);
        }
        else {
            return check_acrossed_pawn_move(start, end);
        }
    }
    
    

    return false;
}

bool Movegen::is_checked(int king_val) const
{
    // int king_val = 0;
    int i = 0, tmp = 0;
    int enemy_knight = (BN - chess_pos.self_player * 7);
    int enemy_rook = (BR - chess_pos.self_player * 7);
    int enemy_pawn = (BP - chess_pos.self_player * 7);
    int enemy_cannon = (BC - chess_pos.self_player * 7);
    int enemy_king = (BK - chess_pos.self_player * 7);
    int count = 0;
    int als_arr[4] = { 1, -1, 16, -16 };

  /*  if (u_overturn) king_val = (1 - chess_pos.self_player) * 7 + 1;
    else king_val = chess_pos.self_player * 7 + 1;*/

    // find your chess pieces
    for (i = 0; i < 256; i++) {
        if (chess_pos.ucpc_Squares[i] == king_val)
            break;
    }

    tmp_king_pos = i;
    // 判断是否被对方的兵(卒)将军
    // 上方 + 左右
    int test = grid_calculate(i, chess_pos.self_player);
    int testval = u_overturn ? 1 - chess_pos.self_player : chess_pos.self_player;
   //  if (chess_pos.ucpc_Squares[grid_calculate(i, chess_pos.self_player)] == (14 - chess_pos.self_player * 7))
    if (chess_pos.ucpc_Squares[grid_calculate(i, testval)] == (14 - chess_pos.self_player * 7))
        return true;
    if (chess_pos.ucpc_Squares[i + 1] == enemy_pawn
        || chess_pos.ucpc_Squares[i - 1] == enemy_pawn)
        return true;


    // 判断是否被对方的马将军(以仕(士)的步长当作马腿)
    // 八面屏风马
    // left - bottom
    if (chess_pos.ucpc_Squares[i + ROW_COUNT - 2] == enemy_knight && chess_pos.ucpc_Squares[i + ROW_COUNT - 2 + 1] == 0)
        return true;

    // left - top
    if (chess_pos.ucpc_Squares[i - ROW_COUNT - 2] == enemy_knight && chess_pos.ucpc_Squares[i - ROW_COUNT - 2 + 1] == 0) 
        return true;
    // top - left
    if (chess_pos.ucpc_Squares[i - 1 - 2 * ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i - 1 - ROW_COUNT] == 0)
        return true;

    // top - right
    if (chess_pos.ucpc_Squares[i + 1 - 2 * ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i + 1 - ROW_COUNT] == 0)
        return true;
    // right - top
    if (chess_pos.ucpc_Squares[i + 2 - ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i + 1 - ROW_COUNT] == 0)
        return true;

    // right - bottom 
    if (chess_pos.ucpc_Squares[i + 2 + ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i + 1 + ROW_COUNT] == 0)
        return true;
    // bottom - right
    if (chess_pos.ucpc_Squares[i + 1 + 2 * ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i + 1 + ROW_COUNT] == 0)
        return true;

    // bottom - left 
    if (chess_pos.ucpc_Squares[i - 1 + 2 * ROW_COUNT] == enemy_knight && chess_pos.ucpc_Squares[i - 1 + ROW_COUNT] == 0)
        return true;

    // 3. 判断是否被对方的车或炮将军(包括将帅对脸)
    for (int m = 0; m < 4; m++) {
        tmp = i;
        count = 0;
        while (is_in_board(tmp)) {
            tmp += als_arr[m];
            if (chess_pos.ucpc_Squares[tmp] != 0) {
                ++count;
                if (chess_pos.ucpc_Squares[tmp] == enemy_rook && count == 1)
                    return true;
                if (chess_pos.ucpc_Squares[tmp] == enemy_cannon && count == 2)
                    return true;
                if (m > 1) {
                    if (chess_pos.ucpc_Squares[tmp] == enemy_king && count == 1)
                        return true;
                }     
            }
        }
    }
    return false;
}

int Movegen::generate_moves(int *move_arr) const {
    int king_move[4] = { 1, -1, 16, -16 };
    int advisor_move[4] = { -17, -15, 15, 17 };
    int bishop_move[4] = { -34, -30, 30, 34 };
    int knight_move[8] = { -33, -31, -18, 14, -14, 18, 31, 33 };
    int rook_move[4] = { 1, -1, 16, -16 };
    int cannon_move[4] = { 1, -1, 16, -16 };
    int pawn_across_the_river_move[2] = { 1, -1 };

    int self_tag = side_tag(chess_pos.self_player);
    int opposite_tag = opposite_side_tag(chess_pos.self_player);
    int tp_pieces, tmp_pieces;
    int move_record = 0;
    for (int start = 51; start < 204; start++) { 
        tp_pieces = chess_pos.ucpc_Squares[start];
        tmp_pieces = 0;
        if (((tp_pieces == 0 ? 0 : tp_pieces + 8) & self_tag) != 0) {  // 己方的子
            switch (tp_pieces - RP > 0 ? tp_pieces - RP : tp_pieces) {
            case RK:
                // int tmp_pieces = 0;
                for (int k = 0; k < 4; k++) {
                    int end = start + king_move[k];
                    if (is_in_palance(end)) { // 是否在九宫格上
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是自己的棋子
                            move_arr[move_record++] = move(start, end);
                    }
                }
                break;
            case RA:
                // int tmp_pieces = 0;
                for (int k = 0; k < 4; k++) {
                    int end = start + advisor_move[k];
                    if (is_in_palance(end)) { // 是否在九宫格上
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是自己的棋子
                            move_arr[move_record++] = move(start, end);
                    }
                }
                break;
            case RB:
                // int tmp_pieces = 0;
                for (int k = 0; k < 4; k++) {
                    int end = start + bishop_move[k];
                    if (is_in_board(end) && is_unacross_the_river(end, chess_pos.self_player) &&
                        isbishop_step_legal(start, end) && isbishop_eye_empty(start, end)) {
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是自己的棋子
                            move_arr[move_record++] = move(start, end);
                    }
                }
                break;
            case RN:
                // int tmp_pieces = 0;
                for (int k = 0; k < 8; k++) {
                    int end = start + knight_move[k];
                    if (is_in_board(end) && isknight_step_legal(start, end)) {
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是自己的棋子
                            move_arr[move_record++] = move(start, end);
                    }
                }
                break;
            case RR:
                // int tmp_pieces = 0;
                for (int k = 0; k < 4; k++) {
                    int end = start + rook_move[k];
                    while (is_in_board(end)) {
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (tmp_pieces == 0) {
                            move_arr[move_record++] = move(start, end);
                        }
                        else {
                            // 如果不是你的子（敌方的子）
                            if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & opposite_tag) != 0) {
                                move_arr[move_record++] = move(start, end);
                            }
                            break;
                        }
                        end += rook_move[k];
                    }  
                }
                break;
            case RC:
                // 一步一步走，碰到第一个不为0的棋子跳过，第二个添加到着法里面
                // int tmp_pieces = 0;
                for (int k = 0; k < 4; k++) {
                    int end = start + cannon_move[k];
                    while (is_in_board(end)) {
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (tmp_pieces == 0) {
                            move_arr[move_record++] = move(start, end);
                        }
                        else break;
                        end += cannon_move[k];
                    }
                    end += cannon_move[k];
                    while (is_in_board(end)) {
                        tmp_pieces = chess_pos.ucpc_Squares[end];
                        if (tmp_pieces != 0) {
                            // 如果不是你的子（敌方的子）
                            if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & opposite_tag) != 0) {
                                move_arr[move_record++] = move(start, end);
                            }
                            break;
                        }
                        end += cannon_move[k];
                    }
                }
                break;
            case RP:
                // int tmp_pieces = 0;
                int end = grid_calculate(start, chess_pos.self_player);
                if (is_in_board(end)) {
                    tmp_pieces = chess_pos.ucpc_Squares[end];
                    if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是己方的子
                    {
                        move_arr[move_record++] = move(start, end);
                    }
                }

                if (u_overturn) {
                    bool test = is_unacross_the_river(start, chess_pos.self_player);
                    std::cout << test << std::endl;
                    if (is_unacross_the_river(start, chess_pos.self_player)) { // 已经过河
                        for (int k = 0; k < 2; k++) {
                            end = start + pawn_across_the_river_move[k];
                            if (is_in_board(end)) {
                                tmp_pieces = chess_pos.ucpc_Squares[end];
                                if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是己方的子
                                {
                                    move_arr[move_record++] = move(start, end);
                                }
                            }
                        }

                    }
                }       
                else {
                    if (_is_across_the_river(start, chess_pos.self_player)) { // 已经过河
                        for (int k = 0; k < 2; k++) {
                            end = start + pawn_across_the_river_move[k];
                            if (is_in_board(end)) {
                                tmp_pieces = chess_pos.ucpc_Squares[end];
                                if (((tmp_pieces == 0 ? 0 : tmp_pieces + 8) & self_tag) == 0) // 不是己方的子
                                {
                                    move_arr[move_record++] = move(start, end);
                                }
                            }
                        }

                    }
                }
                    
                break;
            }  
        }
    }

    return move_record;
}

// 是否被绝杀
bool Movegen::is_mated() const
{
    // 1.思路 ： 先判断将(帅)无论走哪都是被将军
    // 查找全图的自己的子，如果能走并且能抵消掉“被将军”的状态，就不算绝杀 （我没想到该怎么做）

    // 2.思路 ：计算出为什么被绝杀的原因（被什么绝杀）
    // 然后看看自己的子是否能抵消 （难）

    // 3。知道这次被绝杀的子，获取这次的子  （感觉可行）
    // 然后通过挡住这次的子 （有漏洞）

    // 4.获取所有着法，然后判断是否被将军


    int max_moves[MAX_MOVES] = { 0 };
    int move_count = generate_moves(max_moves);

    /*int king_val, tp_pieces;
    if (u_overturn) king_val = (1 - chess_pos.self_player) * 7 + 1;*/
   /* else */

    int king_val = chess_pos.self_player * 7 + 1;
    int tp_pieces;

    for (int i = 0; i < move_count; i++) {
        tp_pieces = mp.MovePiece(max_moves[i]);
        if (!is_checked(king_val)) {
            mp.undo_move_pieces(max_moves[i], tp_pieces);
            return false;
        }
        else mp.undo_move_pieces(max_moves[i], tp_pieces);
    }

    return true;
}

bool Movegen::check_general_pawn_move(int start, int end) const
{
    int start_row = (int)(start >> 4);  // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);      //  (start >> 4) >> 0;   

    int start_col = start % 16;
    int end_col = end % 16;

    if (abs(start_row - end_row) == pawn_step && (start_col - end_col == 0)) {
        // 缺少一个判断不能往后面走的判断
        if (chess_pos.self_player == 0)
            if (u_overturn)
                return start - end < 0;
            else
                return start - end > 0;
           /* return start - end > 0;*/
        if (chess_pos.self_player == 1)
            if (u_overturn)
                return end - start < 0;
            else
                return end - start > 0;
           /* return end - start > 0;*/
        return false;
    }
    else return false;
}

bool Movegen::check_acrossed_pawn_move(int start, int end) const
{
    pawn_vector pdx(pnone);
    int start_row = (int)(start >> 4);  // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);      //  (start >> 4) >> 0;   

    int start_col = start % 16;
    int end_col = end % 16;

    if (start_row - end_row > 0) pdx = p_top; // ↓
    else if (start_row - end_row < 0) pdx = p_bottom; // ↑
    else if (start_col - end_col > 0) pdx = p_left;
    else if (start_col - end_col < 0) pdx = p_right;
    bool test;
    if (pdx == pnone) return false;

    switch (pdx) {
    case p_bottom:
    case p_top:
    case p_left:
    case p_right:
        if (abs(start_row - end_row) + abs(start_col - end_col) != pawn_step) goto failed;
        if (abs(start_row - end_row) == pawn_step && (start_col - end_col == 0) 
            || start_row - end_row == 0 && abs(start_col - end_col) == 1) 
        {
            // 缺少一个判断不能往后面走的判断
            if (chess_pos.self_player == 0) 
            {
                if (u_overturn) {
                    test = start - end < 0 || abs(start - end) == 1;
                    return start - end < 0 || abs(start - end) == 1;
                }
                   
                else {
                    test = start - end > 0;
                    return start - end > 0 || start - end == -1;
                }

            }

            if (chess_pos.self_player == 1) {
                if (u_overturn) {
                    test = start - end > 0 || start - end == -1;
                    return start - end > 0 || start - end == -1;
                }

                else {
                    test = start - end < 0;
                    return start - end < 0 || abs(start - end) == 1;
                }
            }

            else return true;
        }
    }
        
failed:
    return false;
}

bool Movegen::is_same_side(int start, int end) const
{
    // 128 : 1000 0000
   /* int oor = chess_pos.self_player == 0 ? chess_pos.r_oor : chess_pos.b_roor;
    return abs((end >> 4) - oor) <= 4;*/ // // has problems?
    int test = (start ^ end) & 0x80;
    std::cout << test << std::endl;
    return ((start ^ end) & 0x80) == 0;

}

bool Movegen::isbishop_eye_empty(int start, int end) const
{
    return chess_pos.ucpc_Squares[(start + end) >> 1] == 0;
}

bool Movegen::is_straight_line(int start, int end) const // maybe has problems ? rely on grid
{
    int start_grid_x = grid_y(start) - 3;
    int end_grid_x = grid_y(end) - 3;
    int start_grid_y = grid_x(start) - 3;
    int end_grid_y = grid_x(end) - 3;

    if (abs(start_grid_x - end_grid_x) > 0 && (start_grid_y - end_grid_y == 0)) {  // 平移
        return is_halfway_has_another_pieces(start, end, 1);
    }

    if ((start_grid_x - end_grid_x == 0) && (abs(start_grid_y - end_grid_y) > 0)) {  // 垂直
        return is_halfway_has_another_pieces(start, end, 2);
    }
    return false;
}

bool Movegen::is_in_board(int end) const
{
   //  bool truth = piece_limit[end] == 1;
    return piece_limit[end] == 1;
}

bool Movegen::is_halfway_has_another_pieces(int start, int end, int mode) const
{
    rook_vector rdk(none_r);
    int start_row = (int)(start >> 4);  // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);      //  (start >> 4) >> 0;   

    int start_col = start % 16;         
    int end_col = end % 16;

    if (start_row - end_row > 0) rdk = top; // ↓
    else if (start_row - end_row < 0) rdk = bottom; // ↑
    else if (start_col - end_col > 0) rdk = left;
    else if (start_col - end_col < 0) rdk = right;

   //  int val = chess_pos.ucpc_Squares[];

    if (rdk == none_r) return false;

    if (mode == 1) { // 垂直
        switch (rdk) {
        case top:
            for (int i = start_row - 1; i > end_row; i--) 
                if (chess_pos.ucpc_Squares[i * ROW_COUNT + start_col] != 0) goto failed;
            break;
        case bottom:
            for (int i = start_row + 1; i < end_row; i++)
                if (chess_pos.ucpc_Squares[i * ROW_COUNT + start_col] != 0) goto failed;
            break;
        }    
    }
    if (mode == 2) { // 水平
        switch (rdk) {
        case left:
            for (int i = start_col - 1; i > end_col; i--)
                if (chess_pos.ucpc_Squares[start_row * ROW_COUNT + i] != 0) goto failed;
            break;
        case right:
            for(int i = start_col + 1; i < end_col; i++) 
                if (chess_pos.ucpc_Squares[start_row * ROW_COUNT + i] != 0) goto failed;
            break;
        }
    }

    return true;
failed:
    return false;
}

bool Movegen::cannon_determine(int start, int end, int mode) const
{
    rook_vector rdk(none_r);
    int start_row = (int)(start >> 4);  // (start >> 4) >> 0;  
    int end_row = (int)(end >> 4);      //  (start >> 4) >> 0;   

    int start_col = start % 16;
    int end_col = end % 16;

    if (start_row - end_row > 0) rdk = top; // ↓
    else if (start_row - end_row < 0) rdk = bottom; // ↑
    else if (start_col - end_col > 0) rdk = left;
    else if (start_col - end_col < 0) rdk = right;

    int count = 0;
    // 如果终点就是己方的，直接返回false
    // if(chess_pos.ucpc_Squares[end])
    if (rdk == none_k) return false;

    if (mode == 1) { // 垂直
        switch (rdk) {
        case top:
            for (int i = start_row - 1; i > end_row; i--) {
                if (chess_pos.ucpc_Squares[i * ROW_COUNT + start_col] != 0) { // 终点也要判定
                    count++;
                    if (count > 1) goto failed;
                }        
            }
            if (count == 1 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] >= chess_pos.self_player * 7 + 1
                && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] <= chess_pos.self_player * 7 + 7)) // 如果终点是自己的子 
                goto failed;

            if (count == 1 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] == 0)) // 终点没子
                goto failed;
              
            if (count == 0 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] != 0))
                goto failed;
            else return true;

            break;
        case bottom:
            for (int i = start_row + 1; i < end_row; i++) {
                if (chess_pos.ucpc_Squares[i * ROW_COUNT + start_col] != 0) {
                    count++;
                    if (count > 1) goto failed;
                }    
            }          
            if (count == 1 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] >= chess_pos.self_player * 7 + 1
                && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] <= chess_pos.self_player * 7 + 7)) // 如果终点是自己的子 
                goto failed;

            if (count == 1 && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] == 0) goto failed;

            if (count == 0 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] != 0))
                goto failed;
            else return true;

            break;
        }
    }
    if (mode == 2) { // 水平
        switch (rdk) {
        case left:
            for (int i = start_col - 1; i > end_col; i--) {
                if (chess_pos.ucpc_Squares[start_row * ROW_COUNT + i] != 0) {
                    count++;
                    if (count > 1) goto failed;
                }     
            }
            if (count == 1 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] >= chess_pos.self_player * 7 + 1
                && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] <= chess_pos.self_player * 7 + 7)) // 如果终点是自己的子 
                goto failed;
                
            if (count == 1 && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] == 0) goto failed;

            if (count == 0 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] != 0))
                goto failed;
            else return true;

            break;
        case right:
            for (int i = start_col + 1; i < end_col; i++) {
                if (chess_pos.ucpc_Squares[start_row * ROW_COUNT + i] != 0) {
                    count++;
                    if (count > 1) goto failed;
                }
            }

            if (count == 1 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] >= chess_pos.self_player * 7 + 1
                && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] <= chess_pos.self_player * 7 + 7)) // 如果终点是自己的子 
                goto failed;
                
            if (count == 1 && chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] == 0) goto failed;

            if (count == 0 && (chess_pos.ucpc_Squares[end_row * ROW_COUNT + end_col] != 0))
                goto failed;
            else return true;

            break;
        }
    }

    return true;

failed:
    return false;
}

bool Movegen::is_across_the_river(int start, int player) const
{
    //int oor = chess_pos.self_player == 0 ? chess_pos.r_oor : chess_pos.b_roor;
    //return abs((start >> 4) - oor) <= 4; // has problems?
    return (start & 0x80) == (player << 7);
}

bool Movegen::ois_across_the_river(int start, int player) const
{
    return (start & 0x80) != (player << 7);
   //  return false;
}