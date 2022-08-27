#include <Windows.h>
#include "gui.h"
#include "thread.h"
#include "resource.h"

HWND		 main_wnd;
HINSTANCE g_instance;


static int 
exec_main_loop(HINSTANCE hInst, int nCmdShow) {
	MSG msg;
	Main_Gui* main_gui;
	HACCEL accel_table;

	memset(&msg, 0, sizeof(MSG));					// init msg structure
	memset(&accel_table, 0, sizeof(HACCEL));		// init accel table	
	memset(&main_gui, 0, sizeof(Main_Gui));

	g_instance = hInst;
	if (!thread_init()) goto failed;
	if (!main_gui->gui_init(nCmdShow)) goto failed;
	
	accel_table = LoadAccelerators(
		g_instance, MAKEINTRESOURCEW(ID_MAIN_MENU));

	while (GetMessage(&msg, NULL, 0, 0) != FALSE) {
		if (!TranslateAccelerator(msg.hwnd, accel_table, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

failed:
	thread_quit();
	main_gui->quit_main_wnd();
	return(int)msg.wParam;
}


int APIENTRY
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);			// not used
	UNREFERENCED_PARAMETER(lpCmdLine);				// not support cmd-line args, maybe later?
	return exec_main_loop(hInstance, nCmdShow);		// our function
}