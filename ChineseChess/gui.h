#pragma once

#ifndef _GUI_H_INCLUDED
#define _GUI_H_INCLUDED

#include <map>
#include <ShlObj.h>
#include <iostream>

#define  ID_EDIT_CONTROL  9001
#define  ID_STATIC_CONTROL 9002
#define  GUI_TIMER_CLOCK		1
#define  THREAD_INTERLOCK_STEP	1000


const int WINDOWS_STYLES = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX;
const int CHESS_BOARD_WIDTH = 520;
const int CHESS_BOARD_HEIGHT = 576;
const int CHESS_WIDTH = 56;
const int CHESS_HEIGHT = 56;
const int CHESS_BOARD_EDGE = 8;
const int SQUARE_SIZE = 56;
const int MASK_COLOR = RGB(0, 255, 0);
const int EDIT_CONTROL_WIDTH = 200;
const int EDIT_CONTROL_HEIGHT = 550;

extern HWND		 main_wnd;
extern HINSTANCE g_instance;
extern HWND		 edit_control;


enum piece_num 
{
	PIECE_KING = 0,
	PIECE_ADVISOR = 1,
	PIECE_BISHOP = 2,
	PIECE_KNIGHT = 3,
	PIECE_ROOK = 4,
	PIECE_CANNON = 5,
	PIECE_PAWN = 6
};

enum Red_Pieces {
	RK = 1,
	RA = 2,
	RB = 3, 
	RN = 4,
	RR = 5,
	RC = 6,
	RP = 7
};

enum Black_Pieces {
	BK = 8,
	BA = 9,
	BB = 10,
	BN = 11,
	BR = 12,
	BC = 13,
	BP = 14
};

class Main_Gui
{
public:
	Main_Gui() = default;

public:
	int gui_init(int cmd_show);
	int init_main_wnd(int cmd_show);
	HBITMAP load_bitmap_pic(int res_id);
	void quit_main_wnd();
};

// 获得格子的纵坐标
inline int grid_y(int u_flipped) {
	return u_flipped >> 4;
}

// 获得格子的横坐标
inline int grid_x(int u_flipped) {
	return u_flipped & 0x0f;
}



void 
DrawSquare(int selected, HDC hdc_temp, HDC hdc, BYTE flag = FALSE);




#endif
