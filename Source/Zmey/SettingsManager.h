#pragma once
#include <stdint.h>

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
struct SettingsInternalParserHandle
{
	virtual ~SettingsInternalParserHandle()
	{}
};
struct SettingsInternalParserHandleImpl;
class SettingsHandle
{
public:
	SettingsHandle(SettingsInternalParserHandle& internalHandle, const stl::string& section, class SettingsManager& owner);
	~SettingsHandle();

	tmp::string ReadValue(const stl::string& key, const stl::string& defaultValue);
	void WriteValue(const stl::string& key, const stl::string& value);

	bool ReadValue(const stl::string& key, bool defaultValue);
	void WriteValue(const stl::string& key, bool value);

	int32_t ReadValue(const stl::string& key, int32_t defaultValue);
	void WriteValue(const stl::string& key, int32_t value);

	float ReadValue(const stl::string& key, float defaultValue);
	void WriteValue(const stl::string& key, float value);

	tmp::small_vector<tmp::string> ReadValue(const stl::string& key);

	void DeleteValue(const stl::string& key);
private:
	const stl::string m_Section;
	SettingsInternalParserHandleImpl& m_Settings;
	class SettingsManager& m_Owner;
};

class SettingsManager
{
public:
	SettingsManager();
	stl::shared_ptr<SettingsHandle> DataFor(const stl::string& settingName);
private:
	void OnSettingsClosed(const stl::string& settingsName);
	stl::string m_FilePath;
	stl::unique_ptr<SettingsInternalParserHandle> m_Serializer;
	stl::unordered_map<stl::string, stl::shared_ptr<SettingsHandle>> m_OpenSettingsMap;
	friend class SettingsHandle;
};

}