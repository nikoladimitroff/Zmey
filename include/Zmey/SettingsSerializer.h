#pragma once
#include <stdint.h>

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
struct SettingsHandle;
class SettingsSerializer
{
	SettingsSerializer();
	void LoadFromFile(const stl::string& fromFile);
	void SaveToFile(const stl::string& toFile);

	tmp::string ReadValue(const stl::string& section, const stl::string& key, const stl::string& defaultValue);
	void WriteValue(const stl::string& section, const stl::string& key, const stl::string& value);

	bool ReadValue(const stl::string& section, const stl::string& key, bool defaultValue);
	void WriteValue(const stl::string& section, const stl::string& key, bool value);

	int32_t ReadValue(const stl::string& section, const stl::string& key, int32_t defaultValue);
	void WriteValue(const stl::string& section, const stl::string& key, int32_t value);

	float ReadValue(const stl::string& section, const stl::string& key, float defaultValue);
	void WriteValue(const stl::string& section, const stl::string& key, float value);

	void DeleteValue(const stl::string& section, const stl::string& key);

private:
	stl::unique_ptr<SettingsHandle> m_Settings;
};
}