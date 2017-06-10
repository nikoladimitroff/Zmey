#pragma once

#include <Zmey/Config.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{

class ZMEY_API EngineLoop
{
public:
	EngineLoop(class Game* game);
	void Run();
	~EngineLoop();
private:
	class World* m_World;
	class Game* m_Game;
};

}