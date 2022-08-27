#pragma once
#ifndef  _CRULE_H__
#define  _CRULE_H__

const int MAX_MOVES = 256;
const int ROW_COUNT = 16;

const int king_step = 1;
const int advisor_step = 2;
const int knight_step = 3;
const int bishop_step = 4;
const int pawn_step = 1;

extern bool u_overturn;
extern struct MovePieces mp;

enum chess_tag {
	h1

};


struct Movegen {
public:
	bool move_is_legal(int move) const;
	bool is_same_side(int start, int end) const;
	bool isbishop_eye_empty(int start, int end) const;
	bool is_straight_line(int start, int end) const;
	bool is_in_board(int end) const;
	bool is_halfway_has_another_pieces(int start, int end, int mode) const; // only rook
	bool cannon_determine(int start, int end, int mode) const;
	bool is_across_the_river(int start, int player) const; // only directed pawn overturn = false;
	bool ois_across_the_river(int start, int player) const; // overturn = true;

	bool isking_step_legal(int start, int end) const;
	bool isadvisor_step_legal(int start, int end) const;
	bool isbishop_step_legal(int start, int end) const;
	bool isknight_step_legal(int start, int end) const;
	bool is_cannon_step_legal(int start, int end) const;
	bool is_pawn_step_legal(int start, int end) const;
	bool is_checked(int king_val) const;
	bool is_mated() const;

public:
	bool check_general_pawn_move(int start, int end) const;
	bool check_acrossed_pawn_move(int start, int end) const;
	int  generate_moves(int *moves) const;
	
public:
	enum knight_vector {
		none_k = 0,
		top_left = 1,			// 上 - 左
		top_right = 2,			// 上 - 右
		bottom_left = 3,		// 下 - 左
		bottom_right = 4,       // 下 - 右
		left_top = 5,			// 左 - 上 
		left_bottom = 6,		// 左 - 下
		right_top = 7,		    // 右 - 上
		right_bottom = 8		// 右 - 下
	};

	enum rook_vector {
		none_r = 0,
		top = 1,
		bottom = 2,
		left = 3,
		right = 4
	};

	enum pawn_vector {
		pnone = 0,
		p_top = 1,
		p_bottom = 2,
		p_left = 3,
		p_right = 4
	};
};

struct MovePieces {
public:
	int move_pieces(int mv, int& tp_pieces);
	void undo_move_pieces(int mv, int tp_pieces);
	int MovePiece(int mv);
	void undo_move_orgn(int mv, int tp_pieces);
};




#endif // ! _CRULE_h__
