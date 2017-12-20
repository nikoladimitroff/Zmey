#pragma once
#include <Zmey/EngineLoop.h>
#include <Zmey/Game.h>
#include <Zmey/Utilities.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Math/Math.h>

class GiftOfTheSanctumGame : public Zmey::Game
{
public:
	virtual Zmey::Name LoadResources() override;
	virtual void Initialize() override;
	void CastSpell(uint8_t playerIndex);
	virtual void Simulate(float deltaTime) override;
	virtual void Uninitialize() override;
private:
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_WorldName;
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_ScriptName;
	static const uint8_t MaxPlayers = 2;
	Zmey::Vector2 PlayerVel[MaxPlayers];
	Zmey::EntityId m_Players[MaxPlayers];
	float m_CurrentTime = 0.0f;
	uint8_t m_CurrentRing = 5;
};

