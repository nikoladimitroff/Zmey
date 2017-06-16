#include <iostream>
#include <memory>

#include <Zmey/EngineLoop.h>
#include <Zmey/Memory/Allocator.h>
#include <Zmey/Logging.h>
#include <Zmey/Game.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Modules.h>
#include <Zmey/Utilities.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>

class GiftOfTheSanctumGame : public Zmey::Game
{
public:
	virtual Zmey::Name Initialize() override
	{
		m_ScriptName = Zmey::Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");

		for (size_t playerIndex = 0; playerIndex < 2; playerIndex++)
		{
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("movecam"), playerIndex, [playerIndex](float axisValue)
			{
				FORMAT_LOG(Info, Temp, "movecam was called on player %u! axisValue: %f", playerIndex, axisValue);
			});
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("jump"), playerIndex, [playerIndex](float axisValue)
			{
				FORMAT_LOG(Info, Temp, "jump was called on player %u! axisValue: %f", playerIndex, axisValue);
				Zmey::Modules::InputController->Vibrate(playerIndex, 1.f, 0.5f);
			});
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("walk"), playerIndex, [playerIndex](float axisValue)
			{
				FORMAT_LOG(Info, Temp, "walk was called on player %u! axisValue: %f", playerIndex, axisValue);
			});
		}

		m_WorldName = Zmey::Modules::ResourceLoader->LoadResource("IncineratedDataCache/testworld.bin");
		return m_WorldName;
	}
	virtual void Simulate(float deltaTime) override
	{
		if (Zmey::Modules::ResourceLoader->IsResourceReady(m_ScriptName))
		{
			Zmey::Modules::ScriptEngine->ExecuteFromFile(m_ScriptName);
			Zmey::Modules::ResourceLoader->FreeResource(m_ScriptName);
		}
	}
	virtual void Uninitialize() override
	{
	}
private:
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_WorldName;
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_ScriptName;
};


int main()
{
	GiftOfTheSanctumGame game;
	Zmey::EngineLoop loop(&game);
	loop.Run();
	return 0;
}
