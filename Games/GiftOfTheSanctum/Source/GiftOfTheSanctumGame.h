#pragma once
#include <Zmey/EngineLoop.h>
#include <Zmey/Game.h>
#include <Zmey/Utilities.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Components/SpellComponentManager.h>


// TODO: Initialize game loop before game and we can use stl instead std
using EntityVector = std::vector<Zmey::EntityId>;

// TODO: Make component for this
struct HeroCollection
{
	void Resize(size_t size)
	{
		Health.resize(size);
		HealthRegen.resize(size);
		WalkingSpeed.resize(size);
		Entity.resize(size);
		//Spells.resize(size);
	}
	std::vector<float> Health;
	std::vector<float> HealthRegen;
	std::vector<float> WalkingSpeed;
	std::vector<Zmey::EntityId> Entity;
};

class GiftOfTheSanctumGame : public Zmey::Game
{
public:
	virtual Zmey::Name LoadResources() override;
	virtual void Initialize() override;
	void CastSpell(uint8_t playerIndex, uint8_t spellIndex);
	virtual void Simulate(float deltaTime) override;
	virtual void Uninitialize() override;
private:
	void InitializePlayerController(unsigned index);
	void SetupPlayersToSpawnPoints();
	void UpdatePlayers();
	void DoUI();
private:
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_WorldName;
	static const uint8_t MaxPlayers = 2;

	HeroCollection m_Players;
	//SpellCollection m_ActiveSpells;

	std::vector<Zmey::EntityId> m_ForErase;

	EntityVector m_SpawnPoints;
	float m_CurrentTime = 0.0f;
	uint8_t m_CurrentRing = 5;
};

