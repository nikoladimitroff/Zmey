#pragma once

#include <Zmey/Config.h>

namespace Zmey
{

class ZMEY_API EngineLoop
{
public:
	EngineLoop();
	void Run();
	~EngineLoop();
};

}