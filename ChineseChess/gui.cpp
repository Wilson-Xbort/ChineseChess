#include <Windows.h>
#include <iostream>
#include <map>
#include "gui.h"
#include "debug.h"
#include "crule.h"
#include "arr_board.h"
#include "search.h"
#include "thread.h"
#include "resource.h"
#include "editcontrol_append.h"

using namespace search;
using namespace control_text;

static const wchar_t* APP_DEFAULT_NAME = L"Chinese Chess";
static const wchar_t* ABOUT = L"(C) copyright 2020-7 ";

static HBITMAP chess_board;
static HBITMAP chess_pieces[15];
static HBITMAP pieces_selected;
int final_selchesstp; // 最后选中的棋子类型


struct Chess_Position chess_pos;
struct Movegen* movgen = (struct Movegen*)malloc(sizeof(Movegen));
struct Chess_Pieces_Value chess_pv;

HWND static_control, edit_control, main_status_bar;
bool u_overturn;

static void TransparentBlt2(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
	HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc, UINT crTransparent) {
	HDC hImageDC, hMaskDC;
	HBITMAP hOldImageBMP, hImageBMP, hOldMaskBMP, hMaskBMP;

	hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);
	hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);
	hImageDC = CreateCompatibleDC(hdcDest);
	hMaskDC = CreateCompatibleDC(hdcDest);
	hOldImageBMP = (HBITMAP)SelectObject(hImageDC, hImageBMP);
	hOldMaskBMP = (HBITMAP)SelectObject(hMaskDC, hMaskBMP);

	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc) {
		BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	}
	else {
		StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
			hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);
	}
	SetBkColor(hImageDC, crTransparent);
	BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);
	SetBkColor(hImageDC, RGB(0, 0, 0));
	SetTextColor(hImageDC, RGB(255, 255, 255));
	BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);
	SetBkColor(hdcDest, RGB(255, 255, 255));
	SetTextColor(hdcDest, RGB(0, 0, 0));
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
		hMaskDC, 0, 0, SRCAND);
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest,
		hImageDC, 0, 0, SRCPAINT);

	SelectObject(hImageDC, hOldImageBMP);
	DeleteDC(hImageDC);
	SelectObject(hMaskDC, hOldMaskBMP);
	DeleteDC(hMaskDC);
	DeleteObject(hImageBMP);
	DeleteObject(hMaskBMP);
}

inline void DrawTransBmp(HDC hdc, HDC hdcTmp, int xx, int yy, HBITMAP bmp) {
	SelectObject(hdcTmp, bmp);
	TransparentBlt2(hdc, xx, yy, CHESS_WIDTH, CHESS_HEIGHT, hdcTmp, 0, 0, CHESS_WIDTH, CHESS_HEIGHT, MASK_COLOR);
}

void DrawSquare(int selected, HDC hdc_temp, HDC hdc,  BYTE flag) {
	int u_flipped = selected;
	int xx = CHESS_BOARD_EDGE + (grid_x(u_flipped) - 3) * SQUARE_SIZE;
	int yy = CHESS_BOARD_EDGE + (grid_y(u_flipped) - 3) * SQUARE_SIZE;
	SelectObject(hdc_temp, chess_board);
	BitBlt(hdc, xx, yy, SQUARE_SIZE, SQUARE_SIZE, hdc_temp, xx, yy, SRCCOPY);
	int tp_pieces = chess_pos.ucpc_Squares[selected];

	if (tp_pieces != 0) {
		DrawTransBmp(hdc, hdc_temp, xx, yy, chess_pieces[tp_pieces]);
	}
	if (flag) {
		DrawTransBmp(hdc, hdc_temp, xx, yy, pieces_selected);
	}
}

static void gui_chess_event(int mapping, HWND hwnd, int x, int y) {
	HDC hdc = GetDC(hwnd);
	d_hdc = hdc;
	HDC hdc_temp = CreateCompatibleDC(hdc);
	d_hdc_tmp = hdc_temp;
	d_hwnd = hwnd;
	mapping = mapping;

	if (chess_pos.is_mated == true) return;

	int tp_pieces = chess_pos.ucpc_Squares[mapping];
	
	if (((tp_pieces == 0 ? 0 : tp_pieces + 8) & side_tag(chess_pos.self_player)) != 0) {
		final_selchesstp = tp_pieces;
		// 如果点击自己的子，那么直接选中该子
		if (chess_pos.grid_selected != 0) {
			DrawSquare(chess_pos.grid_selected, hdc_temp, hdc);
		}
		chess_pos.grid_selected = mapping;
		DrawSquare(mapping, hdc_temp, hdc ,TRUE);

		if (chess_pos.last_selected != 0) {
			DrawSquare(start_map_arr(chess_pos.last_selected), hdc_temp, hdc);
			DrawSquare(end_map_arr(chess_pos.last_selected), hdc_temp, hdc);
		}

	}
	else if (chess_pos.grid_selected != 0) {
		// 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子	
		chess_pos.last_selected = move(chess_pos.grid_selected, mapping);
		if (movgen->move_is_legal(chess_pos.last_selected)) {
			if (mp.move_pieces(chess_pos.last_selected, tp_pieces)) {
				control_text::cm.selected = chess_pos.last_selected;
				DrawSquare(chess_pos.grid_selected, hdc_temp, hdc, TRUE);
				DrawSquare(mapping, hdc_temp, hdc, TRUE);
				chess_pos.grid_selected = 0; // 移动到AppendForwardORBackward看看
				output_arr_val();
				if (movgen->is_mated()) {
					output_debug_wave(L"gamewin");
					MessageBoxW(hwnd, L"win", L"Congratulations！", MB_OK);
					chess_pos.is_mated = true;
				}
				else {
					int king_val = chess_pos.self_player * 7 + 1;

					movgen->is_checked(king_val) ? output_debug_wave(L"jiangjun") :
						tp_pieces == 0 ? 
						output_debug_wave(L"go") : output_debug_wave(L"eat");

					append_text();  // muti-thread

					Response response;
					response.auto_response_move();
				}
			}
			else
				output_debug_wave(L"goerror");
			
		}
	}

	DeleteDC(hdc_temp);
	ReleaseDC(hwnd, hdc);
}


inline HBITMAP
Main_Gui::load_bitmap_pic(int res_id) {
	return (HBITMAP)LoadImageW(g_instance, MAKEINTRESOURCEW(res_id), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
}

void Main_Gui::quit_main_wnd()
{
	ShowWindow(main_wnd, SW_HIDE);
	if (main_wnd) {
		DestroyWindow(main_wnd);
		UnregisterClass(APP_DEFAULT_NAME, g_instance);
		main_wnd = NULL;
	}
}

static void
draw_chess_board(HWND hwnd, HDC hdc) {
	int coord_x, coord_y, tp_pieces, mapping;
	HDC hdc_tmp = CreateCompatibleDC(hdc);
	SelectObject(hdc_tmp, chess_board);
	BitBlt(hdc, 0, 0, CHESS_BOARD_WIDTH, CHESS_BOARD_HEIGHT, hdc_tmp, 0, 0, SRCCOPY);

	for (int i = 0; i < 9; i++) {
		for (int m = 0; m < 10; m++) {
			coord_x = CHESS_BOARD_EDGE + i * SQUARE_SIZE;
			coord_y = CHESS_BOARD_EDGE + m * SQUARE_SIZE;
	
			mapping = arr_chess_mapping(i, m);
			tp_pieces = chess_pos.ucpc_Squares[mapping];
			if (tp_pieces != 0) {
				DrawTransBmp(hdc, hdc_tmp, coord_x, coord_y, chess_pieces[tp_pieces]);	
			}
			if (tp_pieces == chess_pos.grid_selected && tp_pieces != 0) {
				if (chess_pos.self_player == 0)
					DrawTransBmp(hdc, hdc_tmp, coord_x, coord_y, pieces_selected);
				if (chess_pos.self_player == 1)
					DrawTransBmp(hdc, hdc_tmp, coord_x, coord_y, pieces_selected);
			}		
		}
	}

	DeleteDC(hdc_tmp);
	ReleaseDC(hwnd, hdc);
}

static void 
main_wnd_on_paint(HWND hwnd) {
	HDC hdc;
	PAINTSTRUCT ps;
	hdc = BeginPaint(hwnd, &ps);
	draw_chess_board(hwnd, hdc);
	EndPaint(hwnd, &ps);
}

static void
main_wnd_on_destroy() {
	free(movgen);
	PostQuitMessage(0);
}

static void
main_wnd_on_create(HWND hwnd) {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	rect.right = rect.left + CHESS_BOARD_WIDTH + EDIT_CONTROL_WIDTH + 20;
	rect.bottom = rect.top + CHESS_BOARD_HEIGHT + 30;
	AdjustWindowRect(&rect, WINDOWS_STYLES, TRUE);
	MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	SetTimer(main_wnd, GUI_TIMER_CLOCK, THREAD_INTERLOCK_STEP, NULL);
}

static void 
about_dlg_msg(HWND hwnd) {
	MSGBOXPARAMS mbp;
	MessageBeep(MB_ICONINFORMATION);
	mbp.cbSize = sizeof(MSGBOXPARAMS);
	mbp.hwndOwner = hwnd;
	mbp.hInstance = g_instance;
	mbp.lpszText = ABOUT;
	mbp.lpszCaption = L"About";
	mbp.dwStyle = MB_USERICON;
	mbp.lpszIcon = MAKEINTRESOURCE(IDI_TITLE_ICON);
	mbp.dwContextHelpId = 0;
	mbp.lpfnMsgBoxCallback = NULL;
	mbp.dwLanguageId = 0;
	MessageBoxIndirect(&mbp);
}

static void 
init_new_round(WPARAM wp, HWND hwnd, bool is_overturn = FALSE) {
	u_overturn = is_overturn;
	if(u_overturn) memcpy(chess_pos.ucpc_Squares, overturn_chess_board, sizeof(overturn_chess_board));
	else memcpy(chess_pos.ucpc_Squares, initial_chess_board, sizeof(initial_chess_board));
	
	chess_pos.self_player = 0;	// 0: red  1 : black
	chess_pos.grid_selected = chess_pos.last_selected = 0;
	chess_pos.is_mated = false;
	chess_pv.White_Value = chess_pv.Black_Value = 0;
	d_hdc = d_hdc_tmp = NULL;
	d_hwnd = NULL;
	memset(cm.GetTotal_context(), 0, sizeof(cm.GetTotal_context()));
	/*if (cm.GetMovedPos() != nullptr) {
		wcscpy(cm.GetMovedPos(), L"");
	}
	if (cm.GetMoveStatus() != nullptr) {
		wcscpy(cm.GetMoveStatus(), L"");
	}
	if (cm.GetTP_PIECES() != nullptr) {
		wcscpy(cm.GetTP_PIECES(), L"");
	}
	if (cm.GetUnMovedPos() != nullptr) {
		wcscpy(cm.GetUnMovedPos(), L"");
	}*/

	Inchess_pieces_val();	// 初始化局面值
	
	HDC hdc = GetDC(hwnd);
	draw_chess_board(hwnd, hdc);

	if (u_overturn) {
		HDC ttmp, hhdc;
		hhdc = hdc;
		ttmp = CreateCompatibleDC(hhdc);
		search::Response response;
		response.auto_response_move();
		DeleteDC(ttmp);
	}
	
	ReleaseDC(hwnd, hdc);
}


static void 
main_wnd_on_command(WPARAM wp, HWND hwnd) {
	
	switch (LOWORD(wp)) {
	case ID_START:
	case ID_COMPUTER:   init_new_round(wp, hwnd, TRUE);	break;
	case ID_PEOPLE:     init_new_round(wp, hwnd); break;
	case ID_HELP_ABOUT: about_dlg_msg(hwnd);		break;
	} 
}

static void 
main_wnd_on_left_button_down(LPARAM lp, HWND hwnd) {
	int x = (LOWORD(lp) - CHESS_BOARD_EDGE) / SQUARE_SIZE;
	int y = (HIWORD(lp) - CHESS_BOARD_EDGE) / SQUARE_SIZE;
	std::cout << x << " " << y << std::endl;
	
	if (x >= 0 && x <= 8 && y >= 0 && y <= 9)
		gui_chess_event(arr_chess_mapping(x, y), hwnd, x, y); 
}

wchar_t * 
format_chess_val(int val) {
	static wchar_t buf[6] = { 0 };
	wsprintf(buf, L"%d", val);
	return buf;
}


static void
main_wnd_on_timer() {
	static wchar_t buf[10], buf2[10] = { 0 }, buf3[10] = { 0 };
	wcscpy(buf2, format_chess_val(chess_pv.White_Value));
	wcscpy(buf3, format_chess_val(chess_pv.Black_Value));
	// wcscpy(buf3, calculate_sound_time(ci));
	wsprintf(buf, L"黑棋分 %s | 白棋分 %s ", buf2, buf3);
	SetWindowText(main_status_bar, buf);
}

static void 
main_wnd_on_size(WPARAM wp, LPARAM lp) {
	SendMessageW(main_status_bar, WM_SIZE, wp, lp);
}

static LRESULT CALLBACK
main_wnd_proc(HWND hwnd, UINT message, WPARAM wp, LPARAM lp) {
	switch (message) {
	case WM_DESTROY:	 main_wnd_on_destroy();						break;
	case WM_PAINT:		 main_wnd_on_paint(hwnd);					break;
	case WM_CREATE:		 main_wnd_on_create(hwnd);					break;
	case WM_COMMAND:	 main_wnd_on_command(wp, hwnd);				break;
	case WM_LBUTTONDOWN: main_wnd_on_left_button_down(lp, hwnd);	break;
	case WM_TIMER:		 main_wnd_on_timer();						break;
	case WM_SIZE:		 main_wnd_on_size(wp, lp);						break;
	default:
		return DefWindowProc(hwnd, message, wp, lp);
	}

	return 0;
}


static ATOM
register_wnd_class() {
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = main_wnd_proc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = g_instance;
	wcex.hIcon = LoadIconW(g_instance, MAKEINTRESOURCE(IDI_TITLE_ICON));
	wcex.hCursor = LoadCursorW(NULL, NULL);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wcex.lpszMenuName = MAKEINTRESOURCE(ID_MAIN_MENU);
	wcex.lpszClassName = APP_DEFAULT_NAME;
	wcex.hIconSm = LoadIconW(g_instance, MAKEINTRESOURCE(IDI_TITLE_ICON));
	return RegisterClassExW(&wcex);
}


int Main_Gui::gui_init(int cmd_show) {
	return init_main_wnd(cmd_show);
}

//typedef std::map <int, wchar_t*> CHESS_MAP;
CHESS_MAP chess_map, piece_status, line_num;

const wchar_t *Uchess[7][2] = { // [2][2] => 七行两列, 每一列2个字符
    {L"帅", L"将"}, // 1	2
    {L"士", L"士"}, // 3	4
    {L"相", L"象"}, // 5	6
    {L"马", L"马"}, // 7	8
    {L"车", L"车"}, // 9	10
    {L"炮", L"炮"}, // 11	12
    {L"兵", L"兵"}  // 13	14
};
// int array[3][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}};
const wchar_t *Line_Num[9][2] = {
	{L"1", L"九"},	// 1	2
	{L"2", L"八"},  // 3	4
	{L"3", L"七"},  // 5	6
	{L"4", L"六"},  // 7	8
	{L"5", L"五"},	// 9	10
	{L"6", L"四"},  // 11	12
	{L"7", L"三"},  // 13	14
	{L"8", L"二"},  // 15	16
	{L"9", L"一"}   // 17	18
};

const wchar_t *Pieces_status[3] = { L"进", L"平", L"退" };

inline void init_variable() {
	u_overturn = false;
	int m = 1;
	for (int i = 0; i < 7; i++) {
		for (int k = 0; k < 2; k++) {
			chess_map[m++] = Uchess[i][k];
			/*OutputDebugStringW(chess_map[i]);
			OutputDebugStringA("\n");*/
		}
	}

	int o = 1;
	for (int l = 0; l < 3; l++) {
		piece_status[o++] = Pieces_status[l];
		/*OutputDebugStringA("111\n");
		OutputDebugStringW(piece_status[o - 1]);
		OutputDebugStringA("222\n");
		OutputDebugStringW(Pieces_status[l]);
		OutputDebugStringA("333\n");*/
	}

	int n = 1;
	for (int p = 0; p < 9; p++) {
		for (int u = 0; u < 2; u++)
			line_num[n++] = Line_Num[p][u];
	}
}

int Main_Gui::init_main_wnd(int cmd_show) {

	d_hdc = d_hdc_tmp = NULL;
	d_hwnd = NULL;
	chess_board = load_bitmap_pic(IDB_BOARD);
	pieces_selected = load_bitmap_pic(IDB_BSELECTED);	// black
	for (int i = PIECE_KING; i <= PIECE_PAWN; i++) {
		chess_pieces[i + 1] = load_bitmap_pic(IDB_RK + i);	// 1 ~ 7
		chess_pieces[i + 8] = load_bitmap_pic(IDB_BK + i);	// 8 ~ 14  帅,士,象,马,车,炮,兵
	}

	init_chess_map();
	init_variable();

	register_wnd_class();
	main_wnd = CreateWindowExW(0, APP_DEFAULT_NAME,
		APP_DEFAULT_NAME, WINDOWS_STYLES,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT + 30, NULL, NULL, g_instance, NULL);

	static_control = CreateWindowEx(0, L"STATIC", L"着法列表", WS_CHILD | WS_VISIBLE | SS_CENTER,
		CHESS_BOARD_WIDTH + 80, 0, 60, 15, main_wnd, (HMENU)ID_STATIC_CONTROL, g_instance, NULL);
	SendMessage(static_control, WM_SETFONT, (WPARAM)GetStockObject(17), 0);

	edit_control = CreateWindowEx(0, L"Edit", L"", WS_CHILD | WS_VISIBLE | ES_CENTER |
		ES_AUTOVSCROLL | ES_MULTILINE | ES_WANTRETURN, CHESS_BOARD_WIDTH + 10, 20,
		EDIT_CONTROL_WIDTH, EDIT_CONTROL_HEIGHT, main_wnd, (HMENU)ID_EDIT_CONTROL, g_instance, NULL);
	SendMessage(edit_control, WM_SETFONT, (WPARAM)GetStockObject(17), 0);

	main_status_bar = CreateWindowExW(0, STATUSCLASSNAME,
		NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0, 0, 0, 0, main_wnd, NULL, g_instance, NULL);

	if (!main_wnd || !static_control || !edit_control || !main_status_bar) return 0;
		
	ShowWindow(main_wnd, cmd_show);
	UpdateWindow(main_wnd);
	return (main_wnd == NULL) ? 0 : 1 ;
}