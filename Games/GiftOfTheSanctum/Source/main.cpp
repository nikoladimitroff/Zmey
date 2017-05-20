#include <iostream>
#include <memory>

#include <Zmey/EngineLoop.h>
#include <Zmey/Memory/Allocator.h>


int main()
{
	Zmey::EngineLoop loop("IncineratedDataCache/testworld.bin");
	loop.Run();
	return 0;
}
