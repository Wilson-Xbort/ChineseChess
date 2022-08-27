#pragma once
#ifndef _SEARCH_H_INCLUDED
#define _SEARCH_H_INCLUDED

#define MAX_HISTORY_COUNT 65536

extern HDC d_hdc;
extern HDC d_hdc_tmp;
extern HWND d_hwnd;

namespace search {
	struct Response {
		void auto_response_move();
	};

	struct History_Traversal {
		int c_response;  // �����ߵ���
		int history_table[MAX_HISTORY_COUNT]; // ��ʷ��
	};

	struct Search_Process {
	public:
		void search_main_process();
		int alpha_beta(int depth, int alpha, int beta);

	public:
		int original_distance;  // ��ʼ����
		int timer;	// ��ʱ��
	};

	extern Response response;
	extern History_Traversal histoty_traversal;
	extern Search_Process search_process;
}

#endif // !_SEARCH_H_INCLUDED
