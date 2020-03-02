/*
refernce :
http://myblog.opendocs.co.kr/archives/1206
https://github.com/LimSungMin/IOCompletionPort
*/

#ifndef C_CUSTOM_IOCP
#define C_CUSTOM_IOCP

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <process.h>
#include <iostream>

#define MAX_BUFFER_SIZE 1024

struct stSOCKETINFO
{
	WSAOVERLAPPED	overlapped;
	WSABUF			dataBuf;
	SOCKET			socket;
	char			messageBuffer[MAX_BUFFER_SIZE];
	int				recvBytes;
	int				sendBytes;
};

class CustomIOCP
{
public:
	enum { SERVER_PORT = 12111 };
	enum { SERVER_KERNEL_BACKLOG = 5 };

	CustomIOCP();
	~CustomIOCP();

	bool Init();
	void Run();

	bool CreateWorkerThread();
	void RunWorkerThread_();

private:

	// stSOCKETINFO* pSocketInfo_;
	SOCKET listenSocket_;
	HANDLE* pWorkerHandle_;
	HANDLE hIOCP_;
	bool isRun_;
};

#endif