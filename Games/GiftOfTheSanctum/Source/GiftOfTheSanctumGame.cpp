#include <algorithm>
#include <memory>

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
	const float TOLERANCE = 0.01f;
	if (std::fabsf(value) < TOLERANCE)
		return 0.f;
	return value;
}
}


Zmey::Name GiftOfTheSanctumGame::LoadResources()
{
	m_ScriptName = Zmey::Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");

	m_WorldName = Zmey::Modules::ResourceLoader->LoadResource("IncineratedDataCache/testworld.worldbin");
	return m_WorldName;
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

		PlayerVel[i] = Zmey::Vector2(0.1f, 0.f);
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

void GiftOfTheSanctumGame::CastSpell(uint8_t playerIndex)
{
	auto& transformManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
	auto transform = transformManager.Lookup(m_Players[playerIndex]);
	auto spellId = GetWorld()->SpawnEntity(Zmey::Name("Spell"));
	auto spellTransform = transformManager.Lookup(spellId);
	auto actorForwardVector = spellTransform.Rotation() * Zmey::Vector3(0.f, 1.f, 0.f);
	spellTransform.Position() = transform.Position() + actorForwardVector * 5.f;
}

void GiftOfTheSanctumGame::Simulate(float deltaTime)
{
	if (Zmey::Modules::ResourceLoader->IsResourceReady(m_ScriptName))
	{
		Zmey::Modules::ScriptEngine->ExecuteFromFile(m_ScriptName);
		Zmey::Modules::ResourceLoader->FreeResource(m_ScriptName);
	}

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

