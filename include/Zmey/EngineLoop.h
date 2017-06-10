#pragma once

#include <Zmey/Config.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{

class EngineLoop
{
public:
	ZMEY_API EngineLoop(const char* initialWorld);
	ZMEY_API void Run();
	ZMEY_API ~EngineLoop();
private:
	class World* m_World;
	Zmey::Name m_WorldResource;
};

}