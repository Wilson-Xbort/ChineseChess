#include <ShlObj.h>
#include <Windows.h>
#include "thread.h"
#include "editcontrol_append.h"
#include "arr_board.h"

HANDLE record_text, thread_event;
static long term, tmp, lock;
struct Thread thread;
static CRITICAL_SECTION cs;

static void thread_lock_init() {
	InitializeCriticalSection(&cs);
}

static void thread_lock() {
	EnterCriticalSection(&cs);
}

static void thread_unlock() {
	LeaveCriticalSection(&cs);
}

static void thread_lock_delete() {
	DeleteCriticalSection(&cs);
}

static unsigned long __stdcall
record_proc(void* ctx) {
	while (!term) {
		if (tmp) {
			control_text::cm.AppendChessTypes(final_selchesstp);
			control_text::cm.AppendForwardORBackward(control_text::cm.selected, final_selchesstp);
			control_text::cm.AppendPiecesUnMoved(control_text::cm.selected, final_selchesstp);
			control_text::cm.AppendPiecesMoved(control_text::cm.selected, final_selchesstp);
			control_text::cm.AppendContext();
			InterlockedExchange(&tmp, 0);
			while (lock) WaitForSingleObject(thread_event, INFINITE);
		}	
	}
	
	return (unsigned int)0;
}

void Thread::sync_thread_end() {
	if (record_text != INVALID_HANDLE_VALUE) {
		InterlockedExchange(&term, 1);
		// InterlockedExchange(&limit, 1);
		WaitForSingleObject(record_text, INFINITE);
		CloseHandle(record_text);
		record_text = INVALID_HANDLE_VALUE;
	}
}

int Thread::sync_thread_start() {

	/**
	*  ---------------------------------------------------------------------------------------------------------------------------------------
	*	@brief 	CreateThread 创建线程
	*	@param[1]	LPSECURITY_ATTRIBUTES		lpThreadAttributes		指向SECURITY_ATTRIBUTES型态的结构的指针,NULL使用默认安全性
	*   @param[2]	SIZE_T						dwStackSize				设置初始栈的大小,以字节为单位,如果为0,那么默认将使用与调用该函数的线程相同的栈空间大小
	*   @param[3]	LPTHREAD_START_ROUTINE		lpStartAddress			指向线程函数的指针
	*   @param[4]	LPVOID						lpParameter				向线程函数传递的参数，是一个指向结构的指针，不需传递参数时，为NULL。
	*	@param[5]	DWORD						dwCreationFlags			0：表示创建后立即激活。
	*	@param[6]	LPDWORD						lpThreadId				保存新线程的id。
	*  =======================================================================================================================================
	*  =======================================================================================================================================
	*	返回值：函数成功，返回线程句柄；函数失败返回false。若不想返回线程ID,设置值为NULL。
	*  ---------------------------------------------------------------------------------------------------------------------------------------
	*/
	// InterlockedExchange(&term, 0);

	InterlockedExchange(&term, 0);
	InterlockedExchange(&tmp, 0);
	
	record_text = CreateThread(NULL,
		0, record_proc, NULL, 0, NULL);

	SetThreadPriority(record_text, THREAD_PRIORITY_HIGHEST);
	return 1;
}

void Thread::sync_run() {
	thread_lock();

	// thread.sync_thread_end();
	if (!thread.sync_thread_start()) goto failed;

	thread_unlock();

failed:
	return;
}

void Thread::sync_stop() {
	thread_lock();
	thread.sync_thread_end();
	thread_unlock();
}


int 
thread_init() {
	thread_lock_init();
	thread.sync_run();
	record_text = INVALID_HANDLE_VALUE;
	lock = 0;

	/**
	*	@brief CreateEevnt 用来创建或打开一个命名的或无名的事件对象
	*	@param[1] LPSECURITY_ATTRIBUTES			lpEventAttributes : 安全属性
	*	@param[2] BOOL							bManualReset	  : 复位方式
	*   @param[3] BOOL							bInitialState	  : 初始状态
	*	@param[4] LPCTSTR						lpName			  : 对象名称
	*	==============================================================================================================================================
	*	==============================================================================================================================================
	*	||  @param[1] DETAIL:	确定返回的句柄是否可被子进程继承。如果lpEventAttributes是NULL，此句柄不能被继承。									||
	*	||	@param[2] DETAIL:   指定将事件对象创建成手动复原还是自动复原。如果是TRUE，那么必须用ResetEvent函数来手工将事件的状态复原到无信号状态。	||
	*	||	@param[3] DETAIL:   指定事件对象的初始状态。如果为TRUE，初始状态为有信号状态；否则为无信号状态。										||
	*	||	@param[4] DETAIL:	如果lpName为NULL，将创建一个无名的事件对象。																		||
	*	==============================================================================================================================================
	*	==============================================================================================================================================
	*/
	thread_event = CreateEvent(NULL, TRUE, FALSE, NULL); // 不可继承, 手动复原, 事件初始状态为无信号,无名的事件对象
	return 1;
}

void append_text()
{
	if (!lock) {
		ResetEvent(thread_event);	
		InterlockedExchange(&lock, 1);
		InterlockedExchange(&tmp, 1);
	}
	else {
		SetEvent(thread_event);
		InterlockedExchange(&lock, 0);
		InterlockedExchange(&tmp, 1);
	}
}


void
thread_quit()
{
	thread.sync_stop();
	thread_lock_delete();
}

void msg_callback(int action)
{

}


