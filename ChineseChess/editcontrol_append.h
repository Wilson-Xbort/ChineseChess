#pragma once
#ifndef  _EDIT_CONTROL_H_INCLUDED
#define  _EDIT_CONTROL_H_INCLUDED
#include <ShlObj.h>

extern int final_selchesstp;

namespace control_text {
	extern struct control_msg cm;

	struct control_msg 
	{
	public:
		control_msg() : selected(0)
		{
			Mtp_pieces = Move_status = UnMoved_Pos = Moved_Pos = nullptr; 
		}
		wchar_t* GetTP_PIECES();
		wchar_t* GetMoveStatus();
		wchar_t* GetUnMovedPos();
		wchar_t* GetMovedPos();
		wchar_t* GetTotal_context();
		void AppendChessTypes(int tp_pieces);
		void AppendContext();
		void AppendForwardORBackward(int selected, int tp_pieces);
		void AppendPiecesUnMoved(int selected, int tp_pieces);
		void AppendPiecesMoved(int selected, int tp_pieces);
	public:
		int selected; // 选中的格子
	private:
		// wchar_t* cur_context;
		wchar_t total_context[1024] = { 0 };
		wchar_t* Mtp_pieces;	// 最后合法选中的棋子
		wchar_t* Move_status;	// 移动状态，是“进”,“退”,还是“平”
		wchar_t* UnMoved_Pos;	// 如"马八进三" 中的第二个字 8
		wchar_t* Moved_Pos;		// 如"马八进三" 中的第四个字 三
	};
}


#endif // ! _EDIT_CONTROL_H_INCLUDE
