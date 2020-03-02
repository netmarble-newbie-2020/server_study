#include "custom_iocp.h"

int main() {
	CustomIOCP customIOCP;
	if (customIOCP.Init())
	{
		customIOCP.Run();
	}
	return 0;
}
