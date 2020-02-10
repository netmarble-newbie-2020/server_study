#pragma once

#include "../global_header.hh"

struct UserInfo;

class SingleSimpleServer
{
	static constexpr int RECV_BUFFER_SIZE = 100;

public:
	SingleSimpleServer();
	~SingleSimpleServer();
	
	DISABLED_COPY(SingleSimpleServer)
	DISABLED_MOVE(SingleSimpleServer)

	void InitNetwork();
	void Run();

private:
	SOCKET listenSocket;
	std::atomic<bool> workerThreadLoopFlag;
	_Pos xPos;
};

class SimpleServer
{
	static constexpr int RECV_BUFFER_SIZE = 100;

public:
	SimpleServer();
	~SimpleServer();
	
	DISABLED_COPY(SimpleServer)
	DISABLED_MOVE(SimpleServer)

	void InitNetwork();
	void Run();

	void WorkerThreadFunction(_Key inkey);

private:
	SOCKET listenSocket;
	std::atomic<bool> workerThreadLoopFlag;
	std::list<std::thread> threadCont;

	std::array<std::unique_ptr<UserInfo>, MAX_USER> userCont;
	std::mutex userLock;

	std::queue<_Key> keyPool;
	std::mutex keyLock;
};

namespace ERROR_UTIL
{
	_NORETURN static void Error(const std::string_view msg)
	{
		LPVOID lpMsgBuf;
		int errorCode = WSAGetLastError();
		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf, 0, NULL);

		std::cout << "[" << msg << "(" << errorCode << ")] "
			<< ((LPCTSTR)lpMsgBuf) << "\n";

		LocalFree(lpMsgBuf);

		while (true) {}
	}
}
