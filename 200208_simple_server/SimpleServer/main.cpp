#include "stdafx.h"

#include "SimpleServer.h"

auto main() -> int
{
	std::unique_ptr<SimpleServer> simpleServer
		= std::make_unique<SimpleServer>();

	simpleServer->Run();
}