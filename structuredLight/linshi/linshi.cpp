#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <process.h>
//    User Defines Messages
#define        THRD_MESSAGE_SOMEWORK        WM_USER + 1
#define        THRD_MESSAGE_EXIT            WM_USER + 2
using namespace std;
unsigned _stdcall ThreadProc(LPVOID lpParameter)//线程执行函数
{
	int si = 100;
	MSG msg;
	while (GetMessage(&msg,NULL,0,0)>0)
	{
		switch (msg.message){
			case THRD_MESSAGE_SOMEWORK:
				cout << "THRD_MESSAGE_SOMEWORK" <<endl;
				break;
			case THRD_MESSAGE_EXIT:
				cout << "THRD_MESSAGE_EXIT" << endl;
				break;
		}
	}
	return 0;
}

int main()
{
	int mi = 0;
	unsigned int threadID;
	_beginthreadex(NULL, 0, ThreadProc, NULL, 0, &threadID);//创建一个线程，去执行ThreadProc函数
	while (1){
		PostThreadMessage(threadID, THRD_MESSAGE_SOMEWORK, 0, 0);
		Sleep(100);
	}
	return 0;
}