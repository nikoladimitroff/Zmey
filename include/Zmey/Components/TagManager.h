#pragma once
#include <Zmey/EntityManager.h>
#include <Zmey/Components/ComponentRegistryCommon.h>
#include <Zmey/Components/ComponentManager.h>
#include <Zmey/Hash.h>

namespace Zmey
{

namespace Components
{
	
class TagManager : public ComponentManager
{
	DECLARE_COMPONENT_MANAGER(TagManager);
public:

	virtual void InitializeFromBlob(const tmp::vector<EntityId>&, Zmey::MemoryInputStream&) override;
	virtual void Simulate(float deltaTime) override {}

	bool HasTag(EntityId entity, Zmey::Name tag) const;
	EntityId FindFirstByTag(Zmey::Name tag) const;
	tmp::vector<EntityId> FindAllByTag(Zmey::Name tag) const;

private:
	struct EntityTagPair
	{
		EntityId Entity;
		Name Tag;
		friend bool operator==(const EntityTagPair& lhs, const EntityTagPair& rhs)
		{
			return lhs.Entity == rhs.Entity && lhs.Tag == rhs.Tag;
		}
	};
	stl::vector<EntityTagPair> m_Tags;
};


}

}