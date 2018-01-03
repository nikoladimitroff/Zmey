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
#include <Zmey/Physics/PhysicsComponentManager.h>

#include <imgui/imgui.h>

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
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("WalkX"), index, [this, index](float axisValue, float deltaTime)
		{
			auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players.Entity[index]);
			transform.Position().x += NullifyNearZero(axisValue) * deltaTime * m_Players.WalkingSpeed[index];
		});
	}

	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("WalkZ"), index, [this, index](float axisValue, float deltaTime)
		{
			auto transform = GetWorld()->GetManager<Zmey::Components::TransformManager>().Lookup(m_Players.Entity[index]);
			transform.Position().z += NullifyNearZero(axisValue) * deltaTime * m_Players.WalkingSpeed[index];
		});
	}

	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("Cast"), index, [this, index](float axisValue, float deltaTime)
		{
			CastSpell(index, 0);
		});
	}

}

void GiftOfTheSanctumGame::Initialize()
{
	TEMP_ALLOCATOR_SCOPE;

	// Find the player ids
	auto& tagManager = GetWorld()->GetManager<Zmey::Components::TagManager>();
	m_Players.Resize(MaxPlayers);

	for (uint8_t i = 0u; i < MaxPlayers; i++)
	{
		char buffer[30];
		sprintf_s(buffer, "Player%d", i);
		m_Players.Health[i] = 100.f;
		m_Players.HealthRegen[i] = 0.1f;
		m_Players.WalkingSpeed[i] = 5.f;
		m_Players.Entity[i] = tagManager.FindFirstByTag(Zmey::Name(buffer));
	}

	for (uint8_t playerIndex = 0u; playerIndex < MaxPlayers; playerIndex++)
	{
		InitializePlayerController(playerIndex);
	}

	// Debug setup
	{
		Zmey::Modules.InputController.AddListenerForAction(Zmey::Name("RespawnPlayers"), 0, [this](float axisValue, float deltaTime)
		{
			SetupPlayersToSpawnPoints();
		});
	}

	// Gather spawn points
	Zmey::tmp::vector<Zmey::EntityId> spawnPoints = tagManager.FindAllByTag(Zmey::Name("SpawnPoint"));
	m_SpawnPoints = EntityVector(spawnPoints.begin(), spawnPoints.end());

	// SetupPlayersToSpawnPoints
	SetupPlayersToSpawnPoints();
}

void GiftOfTheSanctumGame::SetupPlayersToSpawnPoints()
{
	auto& pxManager = GetWorld()->GetManager<Zmey::Physics::PhysicsComponentManager>();
	auto& tfManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
	for (uint8_t i = 0u; i < MaxPlayers; i++)
	{
		auto actor = pxManager.Lookup(m_Players.Entity[i]);
		actor->TeleportTo(tfManager.Lookup(m_SpawnPoints[i]).Position());
	}
}

void GiftOfTheSanctumGame::CastSpell(uint8_t playerIndex, uint8_t spellIndex)
{
	auto& transformManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
	auto& pxManager = GetWorld()->GetManager<Zmey::Physics::PhysicsComponentManager>();
	auto transform = transformManager.Lookup(m_Players.Entity[playerIndex]);
	auto spellId = GetWorld()->SpawnEntity(Zmey::Name("Spell"));
	auto spell = pxManager.Lookup(spellId);
	auto actorForwardVector = Zmey::Vector3(0.f, 0.f, 1.f);
	spell->TeleportTo(transform.Position() + actorForwardVector * 3.f);
	spell->ApplyForce(actorForwardVector * 900.f);


	//Append spell instance to active spells
	auto spellManager = GetWorld()->GetManager<SpellComponent>();
	spellManager.Push(SpellComponent::EntryDescriptor{
	1.0f,//InitialSpeed
	1.0f,//ImpactDamage
	1.0f,//InitialMass
	1.0f,//CooldownTime
	1.0f,//ExpireTime
	spellId//EntityId
		});
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

	DoUI();
}

void GiftOfTheSanctumGame::DoUI()
{
	// TODO: This setup is to have transparent window which is not interactable
	// This could be extracted someplace
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_BLACK);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32_BLACK_TRANS);
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::Begin("FPS", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs
	);
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
	ImGui::PopStyleColor(2);

	// Uncomment me for demo of ImGui features
	//ImGui::ShowDemoWindow();
}

void GiftOfTheSanctumGame::Uninitialize()
{
}


