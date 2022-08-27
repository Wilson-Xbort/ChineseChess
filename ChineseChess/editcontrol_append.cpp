#include "gui.h"
#include "arr_board.h"
#include "editcontrol_append.h"
#include <iostream>

namespace control_text {

struct control_msg cm; 


wchar_t* control_msg::GetTP_PIECES()
{
	return Mtp_pieces;
}

wchar_t* control_msg::GetMoveStatus()
{
	return Move_status;
}

wchar_t* control_msg::GetUnMovedPos()
{
	return UnMoved_Pos;
}

wchar_t* control_msg::GetMovedPos()
{
	return Moved_Pos;
}

wchar_t* control_msg::GetTotal_context()
{
	return total_context;
}

void control_msg::AppendContext()
{
	// int m_length = GetWindowTextLength(GetDlgItem(edit_control, ID_EDIT_CONTROL));
	int m_length = GetWindowTextLength(edit_control);
	wchar_t buf1[2] = { 0 }, buf2[2] = { 0 }, buf3[2] = { 0 }, buf4[2] = { 0 }, buf[10] = { 0 };

	wcscpy(buf1, Mtp_pieces); // ��������
	wcscpy(buf2, UnMoved_Pos); // δ�ƶ���λ��
	wcscpy(buf3, Move_status); // �ƶ�״̬
	wcscpy(buf4, Moved_Pos);  // �ƶ����λ��
	wsprintf(buf, L"%s%s%s%s", buf1, buf2, buf3, buf4);
	wsprintf(total_context, L"%s%s\r\n", total_context, buf);
	// memcpy(total_context, tmp_buf, sizeof(total_context));
	/*total_context += cur_context;*/
	/*wsprintf(total_context, L"%s%s%s%s", Mtp_pieces, UnMoved_Pos, Move_status, Moved_Pos);
	const wchar_t* test = L"111";*/
	SendMessageW(edit_control, WM_SETTEXT, 0, (LPARAM)total_context);
}

void control_msg::AppendChessTypes(int tp_pieces)
{
	// ���û�з�ת �� 1 - 7	��ת�˺�Ҳ��һ��
	// �ڷ�	7 - 14	
	// ��chess_map��ӳ�� �� 1 - 2 ˧
	int tmp_pieces = tp_pieces;
	tmp_pieces = tmp_pieces - RP > 0 ? tmp_pieces - RP : tmp_pieces;
	if(tp_pieces > 7)	// move_pieces
		Mtp_pieces = (wchar_t*)chess_map[tmp_pieces * 2];
	else 
		Mtp_pieces = (wchar_t*)chess_map[tmp_pieces * 2 - 1];

	/*OutputDebugStringA("111\n");

	OutputDebugStringW((wchar_t*)chess_map[tp_pieces * 2 - 1]);
	OutputDebugStringA("333\n");
	OutputDebugStringW(Mtp_pieces);
	OutputDebugStringA("222\n");

	for (int i = 1; i < 14; i++) {
		OutputDebugStringW((wchar_t *)chess_map[i]);
	}*/
}

void control_msg::AppendForwardORBackward(int selected, int tp_pieces)
{
	int start_row, start_col, end_row, end_col;
	int start = start_map_arr(selected);
	int end = end_map_arr(selected);

	start_row = start >> 4;
	start_col = start % 16;
	end_row = end >> 4;
	end_col = end & 16;

	if (!u_overturn) {
		if (tp_pieces < 7) // �췽 
		{
			if (start_row - end_row > 0) // ��
			{
				Move_status = (wchar_t *)piece_status[1];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
			else if (start_row - end_row == 0 && abs(start_col - end_col) > 0) // ƽ
			{
				Move_status = (wchar_t*)piece_status[2];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(piece_status[2]);
				OutputDebugStringW(L"ƽ");*/
			}
			else if (start_row - end_row < 0) // ��
			{
				Move_status = (wchar_t*)piece_status[3];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
		}
		else {		// �ڷ�
			if (start_row - end_row > 0) // ��
			{
				Move_status = (wchar_t*)piece_status[1];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
			else if (start_row - end_row == 0 && abs(start_col - end_col) > 0) // ƽ
			{
				Move_status = (wchar_t*)piece_status[2];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(piece_status[2]);
				OutputDebugStringW(L"ƽ");*/
			}
			else if (start_row - end_row < 0) // ��
			{
				Move_status = (wchar_t*)piece_status[3];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
		}
	}
	else {
		if (tp_pieces < 7) // �췽 
		{
			if (start_row - end_row > 0) // ��
			{
				Move_status = (wchar_t*)piece_status[1];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
			else if (start_row - end_row == 0 && abs(start_col - end_col) > 0) // ƽ
			{
				Move_status = (wchar_t*)piece_status[2];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(piece_status[2]);
				OutputDebugStringW(L"ƽ");*/
			}
			else if (start_row - end_row < 0) // ��
			{
				Move_status = (wchar_t*)piece_status[3];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
			
		}
		else {		// �ڷ�
			if (start_row - end_row > 0) // ��
			{
				Move_status = (wchar_t*)piece_status[1];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
			else if (start_row - end_row == 0 && abs(start_col - end_col) > 0) // ƽ
			{
				Move_status = (wchar_t*)piece_status[2];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(piece_status[2]);
				OutputDebugStringW(L"ƽ");*/
			}
			else if (start_row - end_row < 0) // ��
			{
				Move_status = (wchar_t*)piece_status[3];
				/*OutputDebugStringA("3\n");
				OutputDebugStringW(Move_status);
				OutputDebugStringW(L"��");*/
			}
		}
	}
}

void control_msg::AppendPiecesUnMoved(int selected, int tp_pieces)
{
	int start = start_map_arr(selected);
	int start_col = start % 16 - 3 + 1;

	if (!u_overturn) {
		if (tp_pieces < 8) {
			UnMoved_Pos = (wchar_t*)line_num[start_col * 2];
			std::cout << UnMoved_Pos << std::endl;
		}
			
		else
			UnMoved_Pos = (wchar_t*)line_num[start_col * 2 - 1];
	}
	else {
		if (tp_pieces < 8)
			UnMoved_Pos = (wchar_t*)line_num[start_col * 2 - 1];
		else
			UnMoved_Pos = (wchar_t*)line_num[start_col * 2];
	}
}

void control_msg::AppendPiecesMoved(int selected, int tp_pieces)
{
	int end = end_map_arr(selected);
	int start = start_map_arr(selected);
	int start_row = start >> 4;
	int end_row = end >> 4;
	int end_col = end % 16 - 3 + 1;
	int val = abs(end_row - start_row);


	if (!u_overturn) {
		if (tp_pieces < 8) {
			if (Move_status == L"��" || Move_status == L"��")
				Moved_Pos = (wchar_t*)line_num[18 - ((val - 1) * 2)];
			else 
				Moved_Pos = (wchar_t*)line_num[end_col * 2];
		}
			
		else {
			if (Move_status == L"��" || Move_status == L"��")
				Moved_Pos = (wchar_t*)line_num[(18 - ((val - 1) * 2)) - 1 ];
			else 
				Moved_Pos = (wchar_t*)line_num[end_col * 2 - 1];
		}
			
	}
	else {
		if (tp_pieces < 8) {
			if (Move_status == L"��" || Move_status == L"��")
				Moved_Pos = (wchar_t*)line_num[(18 - ((val - 1) * 2)) - 1];
				
			else
				Moved_Pos = (wchar_t*)line_num[end_col * 2 - 1];
		}
			
		else
		{
			if (Move_status == L"��" || Move_status == L"��")
				Moved_Pos = (wchar_t*)line_num[18 - ((val - 1) * 2)];

			else 
				Moved_Pos = (wchar_t*)line_num[end_col * 2];
		}
			
	}
}

}

