#include <algorithm>
#include <memory>
#include <iostream>
#include <GiftOfTheSanctumGame.h>
#include <Zmey/Memory/Allocator.h>
#include <Zmey/Logging.h>
#include <Zmey/Math/Math.h>
#include <Zmey/Modules.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>
#include <Zmey/Components/TagManager.h>

namespace
{
float NullifyNearZero(float value)
{
	const float TOLERANCE = 0.1f;
	if (std::fabsf(value) < TOLERANCE)
		return 0.f;
	return value * 0.1f;
}
}

Zmey::Name GiftOfTheSanctumGame::LoadResources()
{
	m_WorldName = Zmey::Modules.ResourceLoader.LoadResource("IncineratedDataCache/testworld.worldbin");
	return m_WorldName;
}

void GiftOfTheSanctumGame::InitializePlayerController(unsigned index)
{
	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("WalkX"), index, [this, index](float axisValue)
		{
			auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players[index]);
			transform.Position().x += NullifyNearZero(axisValue);
		});
	}

	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("WalkZ"), index, [this, index](float axisValue)
		{
			auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players[index]);
			transform.Position().y += NullifyNearZero(axisValue);
		});
	}

	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("Cast"), index, [this, index](float axisValue)
		{
			CastSpell(index);
		});
	}
}

void GiftOfTheSanctumGame::Initialize()
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
		InitializePlayerController(playerIndex);
	}

	// Gather spawn points
	Zmey::tmp::vector<Zmey::EntityId> spawnPoints = tagManager.FindAllByTag(Zmey::Name("SpawnPoint"));
	m_SpawnPoints = EntityVector(spawnPoints.begin(), spawnPoints.end());
	// Setup player on spawn points
	{
		auto& tfManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
		for (uint8_t i = 0u; i < MaxPlayers; i++)
		{
			auto playerPosition = tfManager.Lookup(m_Players[i]);
			playerPosition.Position() = tfManager.Lookup(m_SpawnPoints[i]).Position();
		}
	}
}

void GiftOfTheSanctumGame::CastSpell(uint8_t playerIndex)
{
	auto& transformManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
	auto transform = transformManager.Lookup(m_Players[playerIndex]);
	auto spellId = GetWorld()->SpawnEntity(Zmey::Name("Spell"));
	auto spellTransform = transformManager.Lookup(spellId);
	auto actorForwardVector = transform.Rotation() * Zmey::Vector3(0.f, 5.f, 0.f);
	spellTransform.Position() = transform.Position() + actorForwardVector * 0.12f;
}

void GiftOfTheSanctumGame::Simulate(float deltaTime)
{
	m_CurrentTime += deltaTime; // TODO: this has a lot of error
	if (m_CurrentTime > 10.0f && m_CurrentRing) // every 10 seconds remove one ring
	{
		m_CurrentTime = 0.0f;
		char buffer[30];
		sprintf_s(buffer, "FloorRing%d", m_CurrentRing);
		auto ring = GetWorld()->GetManager<Zmey::Components::TagManager>().FindFirstByTag(Zmey::Name(buffer));
		GetWorld()->DestroyEntity(ring);
		--m_CurrentRing;
	}
}

void GiftOfTheSanctumGame::Uninitialize()
{
}


