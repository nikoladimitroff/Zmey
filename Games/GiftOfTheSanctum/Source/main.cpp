#include <algorithm>
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
#include <Zmey/Components/TagManager.h>

float NullifyNearZero(float value)
{
	if (std::fabsf(value) < 0.01f)
		return 0.f;
	return value;
}

class GiftOfTheSanctumGame : public Zmey::Game
{
public:
	virtual Zmey::Name LoadResources() override
	{
		m_ScriptName = Zmey::Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");

		m_WorldName = Zmey::Modules::ResourceLoader->LoadResource("IncineratedDataCache/testworld.worldbin");
		return m_WorldName;
	}
	virtual void Initialize() override
	{
		TEMP_ALLOCATOR_SCOPE;

		// Find the player ids
		auto& tagManager = GetWorld()->GetManager<Zmey::Components::TagManager>();
		for (uint8_t i = 0u; i < MaxPlayers; i++)
		{
			char buffer[30];
			sprintf_s(buffer, "Player%d", i);
			m_Players[i] = tagManager.FindFirstByTag(Zmey::Name(buffer));
		}

		for (uint8_t playerIndex = 0u; playerIndex < MaxPlayers; playerIndex++)
		{
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("WalkX"), playerIndex, [this, playerIndex](float axisValue)
			{
				auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players[playerIndex]);
				transform.Position().x += NullifyNearZero(axisValue);
			});
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("WalkZ"), playerIndex, [this, playerIndex](float axisValue)
			{
				auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players[playerIndex]);
				transform.Position().z += NullifyNearZero(axisValue);
			});
			Zmey::Modules::InputController->AddListenerForAction(Zmey::Name("Cast"), playerIndex, [this, playerIndex](float axisValue)
			{
				CastSpell(playerIndex);
			});
		}
	}
	void CastSpell(uint8_t playerIndex)
	{
		auto& transformManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
		auto transform = transformManager.Lookup(m_Players[playerIndex]);
		auto spellId = GetWorld()->SpawnEntity(Zmey::Name("Spell"));
		auto spellTransform = transformManager.Lookup(spellId);
		auto actorForwardVector = spellTransform.Rotation() * Zmey::Vector3(0.f, 1.f, 0.f);
		spellTransform.Position() = transform.Position() + actorForwardVector * 5.f;
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
	static const uint8_t MaxPlayers = 2;
	Zmey::EntityId m_Players[MaxPlayers];
};


int main()
{
	GiftOfTheSanctumGame game;
	Zmey::EngineLoop loop(&game);
	loop.Run();
	return 0;
}
