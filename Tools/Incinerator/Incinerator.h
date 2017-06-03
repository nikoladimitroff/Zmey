#pragma once
#include <string>
#include <map>
#include <vector>
#include <unordered_map>
#include <Zmey/Components/ComponentRegistry.h>

class Incinerator
{
public:
	struct Options
	{
		std::string GameDirectory;
	};

	void Incinerate(const Options&);
private:
	void BuildClassIndex(const std::vector<std::string>& classFiles);
	void IncinerateClass(const std::string& destinationFolder, const std::string& className);
	void IncinerateWorld(const std::string& destinationFolder, const std::string& worldSectionPath);

	struct ComponentEntry : public Zmey::Components::IDataBlob
	{
		ComponentEntry(const std::string& name)
			: ComponentName(name)
		{}

		virtual void WriteData(Zmey::Hash nameHash, const uint8_t* data, uint16_t dataSize) override;

		// The component's name
		std::string ComponentName;
		using SingleDataValue = std::vector<uint8_t>;
		// Map containing each value the component has (e.g. the position, rotation and scale of a transform component)
		std::unordered_map<Zmey::Hash, SingleDataValue> PropertyData;
		// We need the insertion order so store that in an additional vector
		std::vector<Zmey::Hash> PropertyInsertionOrder;
	};
	struct ClassEntry
	{
		std::vector<ComponentEntry> Components;
	};
	std::unordered_map<std::string, ClassEntry> m_ClassIndex;
};

