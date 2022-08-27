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
	*	@brief 	CreateThread �����߳�
	*	@param[1]	LPSECURITY_ATTRIBUTES		lpThreadAttributes		ָ��SECURITY_ATTRIBUTES��̬�Ľṹ��ָ��,NULLʹ��Ĭ�ϰ�ȫ��
	*   @param[2]	SIZE_T						dwStackSize				���ó�ʼջ�Ĵ�С,���ֽ�Ϊ��λ,���Ϊ0,��ôĬ�Ͻ�ʹ������øú������߳���ͬ��ջ�ռ��С
	*   @param[3]	LPTHREAD_START_ROUTINE		lpStartAddress			ָ���̺߳�����ָ��
	*   @param[4]	LPVOID						lpParameter				���̺߳������ݵĲ�������һ��ָ��ṹ��ָ�룬���贫�ݲ���ʱ��ΪNULL��
	*	@param[5]	DWORD						dwCreationFlags			0����ʾ�������������
	*	@param[6]	LPDWORD						lpThreadId				�������̵߳�id��
	*  =======================================================================================================================================
	*  =======================================================================================================================================
	*	����ֵ�������ɹ��������߳̾��������ʧ�ܷ���false�������뷵���߳�ID,����ֵΪNULL��
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
	*	@brief CreateEevnt �����������һ�������Ļ��������¼�����
	*	@param[1] LPSECURITY_ATTRIBUTES			lpEventAttributes : ��ȫ����
	*	@param[2] BOOL							bManualReset	  : ��λ��ʽ
	*   @param[3] BOOL							bInitialState	  : ��ʼ״̬
	*	@param[4] LPCTSTR						lpName			  : ��������
	*	==============================================================================================================================================
	*	==============================================================================================================================================
	*	||  @param[1] DETAIL:	ȷ�����صľ���Ƿ�ɱ��ӽ��̼̳С����lpEventAttributes��NULL���˾�����ܱ��̳С�									||
	*	||	@param[2] DETAIL:   ָ�����¼����󴴽����ֶ���ԭ�����Զ���ԭ�������TRUE����ô������ResetEvent�������ֹ����¼���״̬��ԭ�����ź�״̬��	||
	*	||	@param[3] DETAIL:   ָ���¼�����ĳ�ʼ״̬�����ΪTRUE����ʼ״̬Ϊ���ź�״̬������Ϊ���ź�״̬��										||
	*	||	@param[4] DETAIL:	���lpNameΪNULL��������һ���������¼�����																		||
	*	==============================================================================================================================================
	*	==============================================================================================================================================
	*/
	thread_event = CreateEvent(NULL, TRUE, FALSE, NULL); // ���ɼ̳�, �ֶ���ԭ, �¼���ʼ״̬Ϊ���ź�,�������¼�����
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


