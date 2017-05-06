#include <iostream>
#include <memory>

#include <Zmey/EngineLoop.h>
#include <Zmey/Memory/Allocator.h>


int main()
{
	Zmey::EngineLoop loop;
	loop.Run();
	return 0;
}
