#pragma once
#include <Zmey/EngineLoop.h>
#include <Zmey/Game.h>
#include <Zmey/Utilities.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Math/Math.h>


// TODO: Initialize game loop before game and we can use stl instead std
using EntityVector = std::vector<Zmey::EntityId>;

class GiftOfTheSanctumGame : public Zmey::Game
{
public:
	virtual Zmey::Name LoadResources() override;
	virtual void Initialize() override;
	void CastSpell(uint8_t playerIndex);
	virtual void Simulate(float deltaTime) override;
	virtual void Uninitialize() override;
private:
	void InitializePlayerController(unsigned index);
	void UpdatePlayers();
	void DoUI();
private:
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_WorldName;
	static const uint8_t MaxPlayers = 2;

	Zmey::EntityId m_Players[MaxPlayers];
	EntityVector m_SpawnPoints;
	float m_CurrentTime = 0.0f;
	uint8_t m_CurrentRing = 5;
};

