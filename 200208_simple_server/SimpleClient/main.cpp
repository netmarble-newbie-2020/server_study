#include "stdafx.h"

#include "../global_header.hh"

namespace ERROR_UTIL
{
	_NORETURN static void Error(std::string_view msg)
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
			<< (LPCTSTR)lpMsgBuf << "\n";

		LocalFree(lpMsgBuf);

		while (true) {}
	}
}

void RenderScene(_Key myKey, PACKET::SERVER_TO_CLIENT::ALL_OBJECT_INFO packet)
{
	system("cls"); //콘솔화면 지우기
	// 노가다...

	std::cout << "\n";
	std::cout << "\n";
	std::cout << "\n";
	std::cout << "\n";

	//배열Sort오바지..
	std::vector<std::pair<_Key, _Pos>> userCont;
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (packet.position[i] != -1) { userCont.emplace_back(std::make_pair(i, packet.position[i])); }
	}

	std::sort(userCont.begin(), userCont.end(), [](std::pair<_Key, _Pos> a, std::pair<_Key, _Pos> b) -> bool
	{
		return a.second < b.second;
	});
	
	{
		int index = 0;
		int oldX = -1;
		for (int i = 0; i < MAX_MAP_SIZE; ++i)
		{
			if (userCont[index].second == i)
			{
				// 메인 캐릭터 일정 렌더링 안될 수 있음.. 귀찮아..
				if (userCont[index].first == myKey)
				{
					std::cout << "◎";
				}
				else
				{
					std::cout << "●";
				}

				if (++index == userCont.size()) break;
			}
			else
			{
				std::cout << "  ";
			}
		}
		std::cout << "\n";
	}

	for (int i = 0; i < MAX_MAP_SIZE; ++i)
	{
		std::cout << "■";
	}
	std::cout << "\n";
	std::cout << "pos : " << packet.position[myKey];

	std::cout << "\n";
	std::cout << "\n";
}

int main()
{
	SOCKET mySocket;
	_Key myKey;
	_Pos myXPos;

	// init network
	{
#pragma region [// 윈속 초기화]
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return false;
#pragma endregion

#pragma region [ socket() ]
		if (mySocket = socket(AF_INET, SOCK_STREAM, 0); mySocket == INVALID_SOCKET) ERROR_UTIL::Error("socket()");
#pragma endregion

#pragma region [ connect() ]
		static SOCKADDR_IN serverAddr;
		ZeroMemory(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddr.sin_port = htons(SERVER_LISTEN_PORT_NUMBER);
		int retVal = connect(mySocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
		if (retVal == SOCKET_ERROR) ERROR_UTIL::Error("bind()");
#pragma endregion
	}

	// Recv Add Object once
	{
		PACKET::SERVER_TO_CLIENT::ADD_OBJECT addObject;
		recv(mySocket, reinterpret_cast<char*>(&addObject), sizeof(addObject), 0);
		std::cout << "내 키값은 : " << addObject.key;
		myKey = addObject.key;
		myXPos = addObject.xPos;
	}

	// gameLoop
	while (7)
	{
		rewind(stdin);
		std::cout << "방향을 입력해주세요. a = left, d = right " << std::endl;
		char inputtedChar;
		std::cin >> inputtedChar;

		{
			PACKET::CLIENT_TO_SERVER::MOVE_OBJECT packet;
			packet.dir = inputtedChar == 'a' ? DIRECTION::LEFT : DIRECTION::RIGHT;
			send(mySocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
		}

		{
			PACKET::SERVER_TO_CLIENT::ALL_OBJECT_INFO packet;
			recv(mySocket, reinterpret_cast<char*>(&packet), sizeof(packet), 0);
			myXPos = packet.position[myKey];
			RenderScene(myKey, packet);
		}
	}
}

