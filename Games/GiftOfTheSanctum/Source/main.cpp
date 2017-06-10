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
		m_MeshName = Zmey::Modules::ResourceLoader->LoadResource("Content\\Meshes\\Vampire_A_Lusth\\Vampire_A_Lusth.dae");
		m_ScriptName = Zmey::Modules::ResourceLoader->LoadResource("Content\\Scripts\\main.js");


		Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("movecam"), [](float axisValue)
		{
			FORMAT_LOG(Info, Temp, "movecam was called! axisValue: %f", axisValue);
		});
		Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("jump"), [](float axisValue)
		{
			FORMAT_LOG(Info, Temp, "jump was called! axisValue: %f", axisValue);
		});
		Zmey::Modules::InputController->AddListenerForAction(Zmey::Hash("walk"), [](float axisValue)
		{
			FORMAT_LOG(Info, Temp, "walk was called! axisValue: %f", axisValue);
		});

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

		if (GetWorld()->Meshes.empty() && Zmey::Modules::ResourceLoader->IsResourceReady(m_MeshName))
		{
			auto& entityManager = GetWorld()->GetEntityManager();
			auto newEntity = entityManager.SpawnOne();
			GetWorld()->Meshes.insert(std::make_pair(newEntity, *Zmey::Modules::ResourceLoader->AsMeshHandle(m_MeshName)));
			auto& transformManager = GetWorld()->GetManager<Zmey::Components::TransformManager>();
			transformManager.AddNewEntity(newEntity, Zmey::Vector3(0.0f, -5.0f, 15.0f), Zmey::Vector3(1.0f / 10.0f, 1.0f / 10.f, 1.0f / 10.0f), Zmey::Quaternion(Zmey::Vector3(glm::radians(90.0f), 0.0f, 0.0f)));
		}
	}
	virtual void Uninitialize() override
	{
		Zmey::Modules::ResourceLoader->FreeResource(m_MeshName);
	}
private:
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_WorldName;
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_MeshName;
	Zmey::Utilities::ConstructorInitializable<Zmey::Name> m_ScriptName;
};


int main()
{
	GiftOfTheSanctumGame game;
	Zmey::EngineLoop loop(&game);
	loop.Run();
	return 0;
}
