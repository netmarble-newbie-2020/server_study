#include "stdafx.h"

#include "UserInfo.h"
#include "SimpleServer.h"

SingleSimpleServer::SingleSimpleServer()
	: listenSocket()
	, workerThreadLoopFlag(true)
	, xPos()
{
	InitNetwork();
}

SingleSimpleServer::~SingleSimpleServer()
{
	workerThreadLoopFlag = false;
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2s);
	}
}

void SingleSimpleServer::InitNetwork()
{
#pragma region [ ���� �ʱ�ȭ]
	if (WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
	{ 
		ERROR_UTIL::Error("WSAStartup()");
	}
#pragma endregion

#pragma region [ socket() ]
	if (listenSocket = socket(AF_INET, SOCK_STREAM, 0);
		listenSocket == INVALID_SOCKET)
	{
		ERROR_UTIL::Error("socket()");
	}
#pragma endregion

#pragma region [ Bind() ]
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_LISTEN_PORT_NUMBER);
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("bind()");
	}
#pragma endregion

#pragma region [Print ServerUI]
	PHOSTENT host;
	char name[255];
	char* ip{};

	if (gethostname(name, sizeof(name)) != 0)
	{
		ERROR_UTIL::Error("gethostname()");
	}
	if ((host = gethostbyname(name)) != NULL)
	{
		ip = inet_ntoa(*(struct in_addr*) * host->h_addr_list);
	}

	std::cout << "�������������������������������" << std::endl;
	std::cout << "��  Simple Server	               ��" << std::endl;
	std::cout << "��                                       ver 0.1  200208  ��" << std::endl;
	std::cout << "��                                                        ��" << std::endl;
	std::cout << "��  IP ADDRESS  : " << /*serverAddr.sin_addr.s_addr*/ ip << "                         ��" << std::endl;
	std::cout << "��  PORT NUMBER : " << SERVER_LISTEN_PORT_NUMBER << "                                    ��" << std::endl;
	std::cout << "�������������������������������" << std::endl << std::endl;
}

void SingleSimpleServer::Run()
{
#pragma region [ Listen() ]
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("listen()");
	}
	std::cout << "	- On Listen!,   Server Start! " << std::endl << std::endl;
#pragma endregion

	SOCKET acceptedSocket{};
	SOCKADDR_IN clientAddr{};
	int addrLen{};

	//while (7) 
	//{
		//accept
		addrLen = sizeof(clientAddr);
		if (acceptedSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); 
			acceptedSocket == INVALID_SOCKET) { ERROR_UTIL::Error("accept()"); }

		std::cout << "[ ���ο� Ŭ���̾�Ʈ ���� IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

		xPos = rand() % MAX_MAP_SIZE;
		PACKET::SERVER_TO_CLIENT_SINGLE::ADD_OBJECT addObject{ xPos };
		send(acceptedSocket, reinterpret_cast<char*>(&addObject), sizeof(addObject), 0);

		char recvBuffer[RECV_BUFFER_SIZE];

		while(workerThreadLoopFlag)
		{
			if(recv(acceptedSocket, (char*)&recvBuffer, sizeof(PACKET::CLIENT_TO_SERVER_SINGLE::MOVE_OBJECT), 0) == SOCKET_ERROR)
			{
				ERROR_UTIL::Error("recv");
				break;
			}	

			// ���⵵ Move�� �����.
			switch (static_cast<DIRECTION>(recvBuffer[0]))
			{
				case DIRECTION::LEFT:
				{
					if (xPos != 0) { --xPos; }
					break;
				}
				case DIRECTION::RIGHT:
				{
					if (xPos != MAX_MAP_SIZE) { ++xPos; }
					break;
				}
			}

			PACKET::SERVER_TO_CLIENT_SINGLE::MOVE_OBJECT packet{ xPos };
			std::cout << "[Notify]Ŭ�� " << packet.xPos << "�� �̵��Ͽ����ϴ�. \n";
			send(acceptedSocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
		}
	//}
}


SimpleServer::SimpleServer()
	: listenSocket()
	, workerThreadLoopFlag(true)
	, threadCont()
	, userCont()
	, userLock()
	, keyLock()
	, keyPool()
{
	for (int i = 0; i < MAX_USER; ++i) { keyPool.push(i); }
	for (auto& user : userCont)
	{
		user = std::make_unique<UserInfo>();
		user->posX = -1;
	}

	InitNetwork();
}

SimpleServer::~SimpleServer()
{
	workerThreadLoopFlag = false;
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2s);
	}

	for (auto& thread : threadCont) { thread.join(); };
}

void SimpleServer::InitNetwork()
{
#pragma region [ ���� �ʱ�ȭ]
	if (WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa) != 0) 
	{ 
		ERROR_UTIL::Error("WSAStartup()");
	}
#pragma endregion

#pragma region [ socket() ]
	if (listenSocket = socket(AF_INET, SOCK_STREAM, 0);
		listenSocket == INVALID_SOCKET)
	{
		ERROR_UTIL::Error("socket()");
	}
#pragma endregion

#pragma region [ Bind() ]
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(SERVER_LISTEN_PORT_NUMBER);
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("bind()");
	}
#pragma endregion

#pragma region [Print ServerUI]
	PHOSTENT host;
	char name[255];
	char* ip{};

	if (gethostname(name, sizeof(name)) != 0)
	{
		ERROR_UTIL::Error("gethostname()");
	}
	if ((host = gethostbyname(name)) != NULL)
	{
		ip = inet_ntoa(*(struct in_addr*) * host->h_addr_list);
	}

	std::cout << "�������������������������������" << std::endl;
	std::cout << "��  Simple Server	               ��" << std::endl;
	std::cout << "��                                       ver 0.1  200208  ��" << std::endl;
	std::cout << "��                                                        ��" << std::endl;
	std::cout << "��  IP ADDRESS  : " << /*serverAddr.sin_addr.s_addr*/ ip << "                         ��" << std::endl;
	std::cout << "��  PORT NUMBER : " << SERVER_LISTEN_PORT_NUMBER << "                                    ��" << std::endl;
	std::cout << "�������������������������������" << std::endl << std::endl;
}

void SimpleServer::Run()
{
#pragma region [ Listen() ]
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		ERROR_UTIL::Error("listen()");
	}
	std::cout << "	- On Listen!,   Server Start! " << std::endl << std::endl;
#pragma endregion

	SOCKET acceptedSocket{};
	SOCKADDR_IN clientAddr{};
	int addrLen{};

	while (7) 
	{
		//accept
		addrLen = sizeof(clientAddr);
		if (acceptedSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); 
			acceptedSocket == INVALID_SOCKET) { ERROR_UTIL::Error("accept()"); }

		std::cout << "[ ���ο� Ŭ���̾�Ʈ ���� IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;

		_Key key;
		{
			std::lock_guard<std::mutex> localLock(keyLock);
			if (!keyPool.empty())
			{
				key = keyPool.front();
				keyPool.pop();
				std::cout << "Ű���� " << key << "\n";
			}
			else
			{
				std::cout << "[ MAXCLIENT ������ �źεǾ����ϴ�. IP : " << inet_ntoa(clientAddr.sin_addr) << "  PORT : " << ntohs(clientAddr.sin_port) << "  ] " << std::endl;
				closesocket(acceptedSocket);
				continue;
			}
		}

		_Pos posX = rand() % MAX_MAP_SIZE;
		PACKET::SERVER_TO_CLIENT::ADD_OBJECT addObject{ key, posX };
		send(acceptedSocket, reinterpret_cast<char*>(&addObject), sizeof(addObject), 0);

		{
			std::lock_guard<std::mutex> localLock(userLock);
			userCont[key]->socket = acceptedSocket;
			userCont[key]->posX = posX;
		}

		threadCont.emplace_back([&]() 
		{
			this->WorkerThreadFunction(key);
		});
	}
}

void SimpleServer::WorkerThreadFunction(_Key inkey)
{
	_Key key = inkey;
	SOCKET socket = userCont[key]->socket;
	_Pos& posX = userCont[key]->posX;

	char recvBuffer[RECV_BUFFER_SIZE];

	while (workerThreadLoopFlag)
	{
		// ���� Ŭ�󿡼� ������ �޴� ������ ��Ŷ�� ������Ŷ�̹Ƿ�
		// �������ϱ� ����ٰ� �ٷ� ����
		if(recv(socket, (char*)&recvBuffer, sizeof(PACKET::CLIENT_TO_SERVER::MOVE_OBJECT), 0) == SOCKET_ERROR)
		{
			ERROR_UTIL::Error("recv");
			break;
		}

		// ���⵵ Move�� �����.
		switch (static_cast<DIRECTION>(recvBuffer[0]))
		{
			case DIRECTION::LEFT:
			{
				if (posX != 0) { --posX; }
				break;
			}
			case DIRECTION::RIGHT:
			{
				if (posX != MAX_MAP_SIZE) { ++posX; }
				break;
			}
		}

		std::cout << key << "�� Ŭ�� " << posX << "�� �̵��Ͽ����ϴ�. \n";
		PACKET::SERVER_TO_CLIENT::ALL_OBJECT_INFO packet{};

		for (int i = 0; i < MAX_USER; ++i)
		{
			packet.position[i] = userCont[i]->posX;
		}

		send(socket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
	}

	closesocket(socket);
}
