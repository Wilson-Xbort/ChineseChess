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
		int selected; // ѡ�еĸ���
	private:
		// wchar_t* cur_context;
		wchar_t total_context[1024] = { 0 };
		wchar_t* Mtp_pieces;	// ���Ϸ�ѡ�е�����
		wchar_t* Move_status;	// �ƶ�״̬���ǡ�����,���ˡ�,���ǡ�ƽ��
		wchar_t* UnMoved_Pos;	// ��"��˽���" �еĵڶ����� 8
		wchar_t* Moved_Pos;		// ��"��˽���" �еĵ��ĸ��� ��
	};
}


#endif // ! _EDIT_CONTROL_H_INCLUDE
