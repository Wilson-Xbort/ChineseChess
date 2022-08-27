#include <ShlObj.h>
#include <time.h>
#include "gui.h"
#include "arr_board.h"
#include "crule.h"
#include "search.h"
#include "debug.h"
#include "evaluate.h"

#define MAX_HISTORY_COUNT 65536
#define MAX_SEARCH_DEPTH  64
#define MATED_VALUE		  10000
#define CHESS_DIVING_LINE 9000

HDC d_hdc; 
HDC d_hdc_tmp;
HWND d_hwnd;

namespace search {

History_Traversal histoty_traversal;
Search_Process search_process;

static void init_search_variable() {
	memset(histoty_traversal.history_table, 0, sizeof(histoty_traversal.history_table));
	search_process.original_distance = 0;
	search_process.timer = clock();
}

void Response::auto_response_move()
{
	// Main_Gui main_gui;
	int tp_pieces = 0;
	// HDC hdc = GetDC(main_wnd);
	// HDC hdc_tmp = CreateCompatibleDC(hdc);

	struct Movegen movgen;
	SetCursor((HCURSOR)LoadImage(NULL, IDC_WAIT, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

	search_process.search_main_process();
	SetCursor((HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED));
	mp.move_pieces(histoty_traversal.c_response, tp_pieces);
	// 清除上一步棋的选择标记
	DrawSquare(start_map_arr(chess_pos.last_selected), d_hdc_tmp, d_hdc);
	DrawSquare(end_map_arr(chess_pos.last_selected), d_hdc_tmp, d_hdc);

	// 把电脑走的棋标记出来
	chess_pos.last_selected = histoty_traversal.c_response;
	int start = start_map_arr(chess_pos.last_selected);
	int end = end_map_arr(chess_pos.last_selected);
	int pieces_s = chess_pos.ucpc_Squares[start];
	int pieces_e = chess_pos.ucpc_Squares[end];
	DrawSquare(start, d_hdc_tmp, d_hdc, TRUE);
	DrawSquare(end, d_hdc_tmp, d_hdc, TRUE);

	output_arr_val();
	if (movgen.is_mated()) {
		output_debug_wave(L"gamewin");
		MessageBoxW(main_wnd, L"win", L"Congratulations！", MB_OK);
		chess_pos.is_mated = true;
	}
	else {
		int king_val = chess_pos.self_player * 7 + 1;
		/*if (u_overturn) king_val = (1 - chess_pos.self_player) * 7 + 1;
		else king_val = chess_pos.self_player * 7 + 1;*/
		// 如果没有分出胜负，那么播放将军、吃子或一般走子的声音
		movgen.is_checked(king_val) ? output_debug_wave(L"jiangjun") :
			tp_pieces == 0 ?
			output_debug_wave(L"go") : output_debug_wave(L"eat");
		// exchange_player();
		chess_pos.grid_selected = 0;
		// chess_pos.last_selected = 0;
	}
	DeleteDC(d_hdc_tmp);
	ReleaseDC(d_hwnd, d_hdc);
}

static int compare_val(const void* move_list, const void* history_table) {
	int m = *(int*)history_table;
	int p = *(int*)move_list;
	int o = histoty_traversal.history_table[*(int*)move_list];
	int t = histoty_traversal.history_table[*(int*)history_table];
	// std::cout << o - t << std::endl;
	return histoty_traversal.history_table[*(int*)history_table] - histoty_traversal.history_table[*(int*)move_list];
}

// 迭代加深搜索过程
void Search_Process::search_main_process()
{
	init_search_variable();
	int val;

	// // 迭代加深过程
	for (int i = 1; i <= MAX_SEARCH_DEPTH; i++) {
		val = alpha_beta(i, -MATED_VALUE, MATED_VALUE);
		if (val > CHESS_DIVING_LINE || val < -CHESS_DIVING_LINE) break;	 // 搜索到杀棋停止
		if (clock() - search_process.timer > CLOCKS_PER_SEC) break;  // 搜索超过一秒停止
	}
}

int Search_Process::alpha_beta(int depth, int alpha, int beta)
{
	struct Movegen movgen;
	int tp_pieces = 0;
	int move_list[MAX_MOVES] = { 0 };
	int score = 0;

	// 1. 到达水平线，则返回局面评价值
	if (depth == 0)
		return evaluate();

	// 3.	初始化最佳值和最佳走法
	int vlAlpha = -MATED_VALUE;		// 这样可以知道，是否一个走法都没走过(杀棋)
	int vlBeta = 0;					// 这样可以知道，是否搜索到了Beta走法或PV走法，以便保存到历史表

	// 2. 生成全部走法，并根据历史表排序
	int total_move = movgen.generate_moves(move_list);

	// @param[1] : 待排序数组首地址
	// @param[2] : 数组中待排序元素数量
	// @param[3] : 各元素的占用空间大小
	// @param[4] : 指向函数的指针，用于确定排序的顺序 
	qsort(move_list, total_move, sizeof(int*), compare_val);
	// 4. 逐一走这些着法，递归计算
	for (int i = 0; i < total_move; i++) {
		if (mp.move_pieces(move_list[i], tp_pieces)) {
			score = -alpha_beta(depth - 1, -beta, -alpha);

			mp.undo_move_orgn(move_list[i], tp_pieces);


			// 5. 进行Alpha-Beta大小判断和截断
			if (score > vlAlpha) {				// 找到最佳值(但不能确定是Alpha、PV还是Beta走法)
				vlAlpha = score;				// "vlBest"就是目前要返回的最佳值，可能超出Alpha-Beta边界
				if (score >= beta) {			// 找到一个Beta走法
					vlBeta = move_list[i];		// Beta走法要保存到历史表
					break;						// Beta截断
				}
				if (score > alpha) {			// 找到一个PV走法
					vlBeta = move_list[i];		// PV走法要保存到历史表
					alpha = score;				// 缩小Alpha-Beta边界
				}
			}
		}
	}

	// 5. 所有走法都搜索完了，把最佳走法(不能是Alpha走法)保存到历史表，返回最佳值
	if (vlAlpha == -MATED_VALUE) {
		// 如果是杀棋，就根据杀棋步数给出评价
		return search_process.original_distance - MATED_VALUE;
	}
	if (vlBeta != 0) {
		// 如果不是Alpha走法，就将最佳走法保存到历史表
		histoty_traversal.history_table[vlBeta] += depth * depth;
		if (search_process.original_distance == 0) {
			// 搜索根节点时，总是有一个最佳走法(因为全窗口搜索不会超出边界)，将这个走法保存下来
			histoty_traversal.c_response = vlBeta;
		}
	}
	return vlAlpha;
}

}


