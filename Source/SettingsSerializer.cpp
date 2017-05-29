#include <Zmey/SettingsSerializer.h>

#include <cpptoml/cpptoml.h>

namespace Zmey
{
SettingsSerializer::SettingsSerializer() {}
void SettingsSerializer::LoadFromFile(const stl::string& fromFile)
{
	m_MiniIni = new minIni(fromFile);
}

void SettingsSerializer::SaveToFile(const stl::string& toFile)
{
}

tmp::string SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, const stl::string& defaultValue);
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, const stl::string& value);

bool SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, bool defaultValue);
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, bool value);

int32_t SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, int32_t defaultValue);
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, int32_t value);

void SettingsSerializer::DeleteValue(const stl::string& section, const stl::string& key);
void SettingsSerializer::DeleteSection(const stl::string& section);

	}