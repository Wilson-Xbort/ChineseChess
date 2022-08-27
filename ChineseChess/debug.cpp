#include <ShlObj.h>
#include <iostream>
#include <fstream>
#include "debug.h"
#include "arr_board.h"

static wchar_t dir[MAX_PATH];
static wchar_t debug_info[MAX_PATH];
static wchar_t wav_info[MAX_PATH];

void 
output_arr_val()
{
	const wchar_t* filename = L"debug_file";
	GetModuleFileNameW(
		NULL, dir, _MAX_PATH);		// 获取路径(root path)
	wchar_t* p = wcsrchr(dir, L'\\');		// Scan a string for the last occurrence of a character
	if (p) *p = L'\0';
	wsprintf(debug_info, L"%s\\%s\\%s.txt", dir, filename, filename);		// 格式化
	OutputDebugStringW(debug_info);

	std::wfstream in(debug_info);
	for (int i = 0; i < 256; i++) {
		if ((i % 16) == 0) in << std::endl;
		if(chess_pos.ucpc_Squares[i] < 10) 
			in << chess_pos.ucpc_Squares[i] << "      ";
		else in << chess_pos.ucpc_Squares[i] << "     ";
	}
}


void
output_debug_wave(const wchar_t* filename) {
	// const wchar_t* filename = L"Man_jiangjun";
	wchar_t* p = wcsrchr(dir, L'\\');		// Scan a string for the last occurrence of a character
	if (p) *p = L'\0';
	wsprintf(wav_info, L"%s\\dist\\sounds\\%s.wav", dir, filename);		// 格式化
	PlaySoundW(wav_info, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
}