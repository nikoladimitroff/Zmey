#include <Zmey/Components/RectangleManager.h>

#include <nlohmann/json.hpp>

#include <Zmey/MemoryStream.h>
#include <Zmey/Components/ComponentRegistry.h>
#include <Zmey/World.h>
#include <Zmey/Components/TransformManager.h>

namespace Zmey
{
namespace Components
{

void RectangleManager::InitializeFromBlob(const tmp::vector<EntityId>& entities, Zmey::MemoryInputStream&)
{
	m_Entities.reserve(m_Entities.size() + entities.size());
	std::copy(entities.begin(), entities.end(), std::back_inserter(m_Entities));
	m_Rects.reserve(m_Rects.size() + entities.size());
	for (EntityId::IndexType i = 0; i < entities.size(); i++)
	{
		Graphics::Rect rect{ 0.f, 0.f, 0.1f, 0.1f, { (float)rand() / RAND_MAX, (float)rand() / RAND_MAX , (float)rand() / RAND_MAX , 1.f} };
		m_Rects.push_back(std::move(rect));
	}
}
void RectangleManager::Simulate(float deltaTime)
{
	auto& transformManager = GetWorld().GetManager<TransformManager>();
	for (EntityId::IndexType i = 0u; i < m_Entities.size(); i++)
	{
		auto transform = transformManager.Lookup(m_Entities[i]);

		transform.Position() += Vector3(0.05f, 0.05f, 0.05f) * deltaTime;

		m_Rects[i].x = transform.Position().x;
		m_Rects[i].y = transform.Position().y;
	}
}

const stl::vector<Graphics::Rect>& RectangleManager::GetRectsToRender() const
{
	return m_Rects;
}


DEFINE_COMPONENT_MANAGER(RectangleManager, Rectangle, &Zmey::Components::EmptyDefaultsToBlobImplementation, &Zmey::Components::EmptyToBlobImplementation);

}
}