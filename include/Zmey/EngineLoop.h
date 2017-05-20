#pragma once

#include <Zmey/Config.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{

class ZMEY_API EngineLoop
{
public:
	EngineLoop(const char* initialWorld);
	void Run();
	~EngineLoop();
private:
	class World* m_World;
	ResourceId m_WorldResourceId;
};

}