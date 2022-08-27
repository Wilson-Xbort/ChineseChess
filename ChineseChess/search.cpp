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
	// �����һ�����ѡ����
	DrawSquare(start_map_arr(chess_pos.last_selected), d_hdc_tmp, d_hdc);
	DrawSquare(end_map_arr(chess_pos.last_selected), d_hdc_tmp, d_hdc);

	// �ѵ����ߵ����ǳ���
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
		MessageBoxW(main_wnd, L"win", L"Congratulations��", MB_OK);
		chess_pos.is_mated = true;
	}
	else {
		int king_val = chess_pos.self_player * 7 + 1;
		/*if (u_overturn) king_val = (1 - chess_pos.self_player) * 7 + 1;
		else king_val = chess_pos.self_player * 7 + 1;*/
		// ���û�зֳ�ʤ������ô���Ž��������ӻ�һ�����ӵ�����
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

// ����������������
void Search_Process::search_main_process()
{
	init_search_variable();
	int val;

	// // �����������
	for (int i = 1; i <= MAX_SEARCH_DEPTH; i++) {
		val = alpha_beta(i, -MATED_VALUE, MATED_VALUE);
		if (val > CHESS_DIVING_LINE || val < -CHESS_DIVING_LINE) break;	 // ������ɱ��ֹͣ
		if (clock() - search_process.timer > CLOCKS_PER_SEC) break;  // ��������һ��ֹͣ
	}
}

int Search_Process::alpha_beta(int depth, int alpha, int beta)
{
	struct Movegen movgen;
	int tp_pieces = 0;
	int move_list[MAX_MOVES] = { 0 };
	int score = 0;

	// 1. ����ˮƽ�ߣ��򷵻ؾ�������ֵ
	if (depth == 0)
		return evaluate();

	// 3.	��ʼ�����ֵ������߷�
	int vlAlpha = -MATED_VALUE;		// ��������֪�����Ƿ�һ���߷���û�߹�(ɱ��)
	int vlBeta = 0;					// ��������֪�����Ƿ���������Beta�߷���PV�߷����Ա㱣�浽��ʷ��

	// 2. ����ȫ���߷�����������ʷ������
	int total_move = movgen.generate_moves(move_list);

	// @param[1] : �����������׵�ַ
	// @param[2] : �����д�����Ԫ������
	// @param[3] : ��Ԫ�ص�ռ�ÿռ��С
	// @param[4] : ָ������ָ�룬����ȷ�������˳�� 
	qsort(move_list, total_move, sizeof(int*), compare_val);
	// 4. ��һ����Щ�ŷ����ݹ����
	for (int i = 0; i < total_move; i++) {
		if (mp.move_pieces(move_list[i], tp_pieces)) {
			score = -alpha_beta(depth - 1, -beta, -alpha);

			mp.undo_move_orgn(move_list[i], tp_pieces);


			// 5. ����Alpha-Beta��С�жϺͽض�
			if (score > vlAlpha) {				// �ҵ����ֵ(������ȷ����Alpha��PV����Beta�߷�)
				vlAlpha = score;				// "vlBest"����ĿǰҪ���ص����ֵ�����ܳ���Alpha-Beta�߽�
				if (score >= beta) {			// �ҵ�һ��Beta�߷�
					vlBeta = move_list[i];		// Beta�߷�Ҫ���浽��ʷ��
					break;						// Beta�ض�
				}
				if (score > alpha) {			// �ҵ�һ��PV�߷�
					vlBeta = move_list[i];		// PV�߷�Ҫ���浽��ʷ��
					alpha = score;				// ��СAlpha-Beta�߽�
				}
			}
		}
	}

	// 5. �����߷����������ˣ�������߷�(������Alpha�߷�)���浽��ʷ���������ֵ
	if (vlAlpha == -MATED_VALUE) {
		// �����ɱ�壬�͸���ɱ�岽����������
		return search_process.original_distance - MATED_VALUE;
	}
	if (vlBeta != 0) {
		// �������Alpha�߷����ͽ�����߷����浽��ʷ��
		histoty_traversal.history_table[vlBeta] += depth * depth;
		if (search_process.original_distance == 0) {
			// �������ڵ�ʱ��������һ������߷�(��Ϊȫ�����������ᳬ���߽�)��������߷���������
			histoty_traversal.c_response = vlBeta;
		}
	}
	return vlAlpha;
}

}


