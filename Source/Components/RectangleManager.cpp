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
	m_Entities.assign(entities.begin(), entities.end());
	m_Rects.reserve(entities.size());
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
		m_Rects[i].x = transform.Position().x;
		m_Rects[i].y = transform.Position().y;
	}
}

tmp::vector<Graphics::Rect> RectangleManager::GetRectsToRender() const
{
	tmp::vector<Graphics::Rect> rects(m_Rects.begin(), m_Rects.end());
	return rects;
}


DEFINE_COMPONENT_MANAGER(RectangleManager, Rectangle, &Zmey::Components::EmptyDefaultsToBlobImplementation, &Zmey::Components::EmptyToBlobImplementation);

}
}