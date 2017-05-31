#include <Zmey/SettingsSerializer.h>

#include <simpleini/SimpleIni.h>

namespace Zmey
{
struct SettingsHandle
{
	SettingsHandle()
		: SimpleIni(true, false, true)
	{}
	CSimpleIniCaseA SimpleIni;
};
SettingsSerializer::SettingsSerializer() {}
void SettingsSerializer::LoadFromFile(const stl::string& fromFile)
{
	m_Settings.reset(new SettingsHandle());
	m_Settings->SimpleIni.LoadFile(fromFile.c_str());
}

void SettingsSerializer::SaveToFile(const stl::string& toFile)
{
	m_Settings->SimpleIni.SaveFile(toFile.c_str(), true);
}

tmp::string SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, const stl::string& defaultValue)
{
	return m_Settings->SimpleIni.GetValue(section.c_str(), key.c_str(), defaultValue.c_str());
}
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, const stl::string& value)
{
	m_Settings->SimpleIni.SetValue(section.c_str(), key.c_str(), value.c_str());
}

bool SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, bool defaultValue)
{
	return m_Settings->SimpleIni.GetBoolValue(section.c_str(), key.c_str(), defaultValue);
}
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, bool value)
{
	m_Settings->SimpleIni.SetBoolValue(section.c_str(), key.c_str(), value);
}

int32_t SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, int32_t defaultValue)
{
	return m_Settings->SimpleIni.GetLongValue(section.c_str(), key.c_str(), defaultValue);
}
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, int32_t value)
{
	m_Settings->SimpleIni.SetLongValue(section.c_str(), key.c_str(), value);
}

float SettingsSerializer::ReadValue(const stl::string& section, const stl::string& key, float defaultValue)
{
	return static_cast<float>(m_Settings->SimpleIni.GetDoubleValue(section.c_str(), key.c_str(), defaultValue));
}
void SettingsSerializer::WriteValue(const stl::string& section, const stl::string& key, float value)
{
	m_Settings->SimpleIni.SetDoubleValue(section.c_str(), key.c_str(), value);
}

void SettingsSerializer::DeleteValue(const stl::string& section, const stl::string& key)
{
	m_Settings->SimpleIni.Delete(section.c_str(), key.c_str(), true);
}

}