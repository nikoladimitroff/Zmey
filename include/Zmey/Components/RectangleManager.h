#pragma once
#include <Zmey/Math/Math.h>
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>
#include <Zmey/Graphics/FrameData.h>

namespace Zmey
{

namespace Components
{

class RectangleManager : public ComponentManager
{
	DECLARE_COMPONENT_MANAGER(RectangleManager);
public:

	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override;

	tmp::vector<Graphics::Rect> GetRectsToRender() const;
private:
	stl::vector<EntityId> m_Entities;
	stl::vector<Graphics::Rect> m_Rects;
};

}

}