#pragma once
#include <Zmey/Hash.h>

namespace Zmey
{

class Game
{
public:
	Game()
		: m_World(nullptr)
	{}
	virtual ~Game() {}
	// Initializes the game and returns the name of the initial world
	virtual Zmey::Name Initialize() = 0;
	virtual void Simulate(float deltaTime) = 0;
	virtual void Uninitialize() = 0;
	class World* GetWorld()
	{
		return m_World;
	}

private:
	void SetWorld(class World* world)
	{
		m_World = world;
	}
	class World* m_World;
	friend class EngineLoop;
};

}