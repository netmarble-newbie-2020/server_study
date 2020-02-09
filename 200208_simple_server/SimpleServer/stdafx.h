#pragma once

// C++
#include <iostream>

#include <string>
#include <string_view>

#include <list>
#include <array>
#include <queue>

#include <thread>
#include <atomic>
#include <mutex>

#pragma region [ For Window Network ]
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "wininet.lib")
#include <WinSock2.h>
#pragma endregion

