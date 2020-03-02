#include "custom_iocp.h"

unsigned int WINAPI CallWorkerThread(LPVOID p)
{
	CustomIOCP* pOverlappedEvent = (CustomIOCP*)p;
	pOverlappedEvent->RunWorkerThread_();
	return 0;
}

CustomIOCP::CustomIOCP() :
	isRun_(false), listenSocket_(INVALID_SOCKET)
	, pWorkerHandle_(nullptr), hIOCP_(NULL)
{
}

CustomIOCP::~CustomIOCP()
{
	isRun_ = false;
	closesocket(listenSocket_);
	WSACleanup();
}

// Init Listen Socket
bool CustomIOCP::Init()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		std::cerr << "[ERROR] WSAStartup\n";
		return false;
	}

	listenSocket_ = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket_ == INVALID_SOCKET)
	{
		std::cerr << "[ERROR] WSASocket\n";
		return false;
	}

	// set IPv4, address, port
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// binding listen socket
	if (bind(listenSocket_, (struct sockaddr*) & serverAddr, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		std::cerr << "[ERROR] bind\n";
		closesocket(listenSocket_);
		WSACleanup();
		return false;
	}

	// ready to accept
	if (listen(listenSocket_, SERVER_KERNEL_BACKLOG) == SOCKET_ERROR)
	{
		std::cerr << "[ERROR] listen\n";
		closesocket(listenSocket_);
		WSACleanup();
		return false;
	}
	return true;
}

// Accept Client & Assign Client Socket Handle to Completion Port
void CustomIOCP::Run()
{
	isRun_ = true;

	// Create Completion Port
	hIOCP_ = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (!CreateWorkerThread())
	{
		return;
	}

	// client info
	stSOCKETINFO* pSocketInfo;
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	SOCKET clientSocket;
	DWORD recvBytes;
	DWORD flags;

	while (isRun_) {
		// Accept Client
		clientSocket = WSAAccept(
			listenSocket_, (struct sockaddr*) & clientAddr, &addrLen, NULL, NULL
		);

		if (clientSocket == INVALID_SOCKET)
		{
			std::cerr << "[ERROR] WSAAccept\n";
			return;
		}

		// make stSOCKETINFO wrapping accepted client
		pSocketInfo = new stSOCKETINFO();
		pSocketInfo->socket = clientSocket;
		pSocketInfo->recvBytes = 0;
		pSocketInfo->sendBytes = 0;
		pSocketInfo->dataBuf.len = MAX_BUFFER_SIZE;
		pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
		flags = 0;

		// assign client socket handle to completion port
		hIOCP_ = CreateIoCompletionPort(
			(HANDLE)clientSocket, hIOCP_, (DWORD)pSocketInfo, 0
		);

		// assign overlapped io socket
		int retWSARecv = WSARecv(
			pSocketInfo->socket,
			&pSocketInfo->dataBuf,
			1,
			&recvBytes,
			&flags,
			(LPWSAOVERLAPPED) & (pSocketInfo->overlapped),
			NULL
		);
		if (retWSARecv == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			std::cerr << "[ERROR] WSARecv\n";
			return;
		}
	}
}

bool CustomIOCP::CreateWorkerThread()
{
	unsigned int threadId;

	// thread cnt = cpu core * 2
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	int nThread = sysInfo.dwNumberOfProcessors * 2;

	// thread handler
	pWorkerHandle_ = new HANDLE[nThread];
	// thread create
	for (int i = 0; i < nThread; i++)
	{
		pWorkerHandle_[i] = (HANDLE*)_beginthreadex(
			NULL, 0, &CallWorkerThread, this, CREATE_SUSPENDED, &threadId
		);
		if (pWorkerHandle_[i] == NULL)
		{
			std::cerr << "[ERROR] can't create worker thread\n";
			return false;
		}
		ResumeThread(pWorkerHandle_[i]);
	}
	return true;
}

void CustomIOCP::RunWorkerThread_()
{
	// data size
	DWORD recvBytes;
	DWORD sendBytes;

	// Completion Key pointer
	stSOCKETINFO* pCompletionKey;

	// Overlapped struct pointer
	stSOCKETINFO* pSocketInfo;
	// WSARecv flags
	DWORD flags;

	while (isRun_) {
		// wait at WaitingThreadQueue (IOCP)
		if (GetQueuedCompletionStatus(hIOCP_,
			&recvBytes,
			(LPDWORD)&pCompletionKey,
			(LPOVERLAPPED*)&pSocketInfo,
			INFINITE) == 0)
		{
			std::cerr << "GetQueuedCompletionStatus\n";
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}

		pSocketInfo->dataBuf.len = recvBytes;

		if (recvBytes == 0)
		{
			// client disconnected;
			closesocket(pSocketInfo->socket);
			free(pSocketInfo);
			continue;
		}
		else
		{
			// buffer : pSocketInfo->dataBuf.buf
			// len    : pSocketInfo->dataBuf.len
			std::cout << pSocketInfo->dataBuf.buf << ' ' << pSocketInfo->dataBuf.len << '\n';

			// TODO : some action

			// init stSOCKETINFO
			ZeroMemory(pSocketInfo->messageBuffer, MAX_BUFFER_SIZE);
			pSocketInfo->recvBytes = 0;
			pSocketInfo->sendBytes = 0;
			ZeroMemory(&(pSocketInfo->overlapped), sizeof(OVERLAPPED));
			pSocketInfo->dataBuf.len = MAX_BUFFER_SIZE;
			pSocketInfo->dataBuf.buf = pSocketInfo->messageBuffer;
			flags = 0;

			// thread recv again
			int retWSARecv = WSARecv(
				pSocketInfo->socket,
				&pSocketInfo->dataBuf,
				1,
				&recvBytes,
				&flags,
				(LPWSAOVERLAPPED) & (pSocketInfo->overlapped),
				NULL
			);
			if (retWSARecv == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				std::cerr << "[ERROR] WSARecv\n";
				return;
			}
		}
	}
}
