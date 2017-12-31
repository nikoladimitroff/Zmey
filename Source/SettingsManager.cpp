#include <Zmey/SettingsManager.h>

#include <simpleini/SimpleIni.h>
#include <Zmey/Utilities.h>

namespace Zmey
{
struct SettingsInternalParserHandleImpl : public SettingsInternalParserHandle
{
	SettingsInternalParserHandleImpl()
		: SimpleIni(true, true, true)
	{}
	CSimpleIniCaseA SimpleIni;
};
SettingsHandle::SettingsHandle(SettingsInternalParserHandle& internalHandle, const stl::string& section, class SettingsManager& owner)
	: m_Section(section)
	, m_Settings(static_cast<SettingsInternalParserHandleImpl&>(internalHandle))
	, m_Owner(owner)
{}
SettingsHandle::~SettingsHandle()
{
	m_Owner.OnSettingsClosed(m_Section);
}

tmp::string SettingsHandle::ReadValue(const stl::string& key, const stl::string& defaultValue)
{
	return m_Settings.SimpleIni.GetValue(m_Section.c_str(), key.c_str(), defaultValue.c_str());
}
void SettingsHandle::WriteValue(const stl::string& key, const stl::string& value)
{
	m_Settings.SimpleIni.SetValue(m_Section.c_str(), key.c_str(), value.c_str());
}

bool SettingsHandle::ReadValue(const stl::string& key, bool defaultValue)
{
	return m_Settings.SimpleIni.GetBoolValue(m_Section.c_str(), key.c_str(), defaultValue);
}
void SettingsHandle::WriteValue(const stl::string& key, bool value)
{
	m_Settings.SimpleIni.SetBoolValue(m_Section.c_str(), key.c_str(), value);
}

int32_t SettingsHandle::ReadValue(const stl::string& key, int32_t defaultValue)
{
	return m_Settings.SimpleIni.GetLongValue(m_Section.c_str(), key.c_str(), defaultValue);
}
void SettingsHandle::WriteValue(const stl::string& key, int32_t value)
{
	m_Settings.SimpleIni.SetLongValue(m_Section.c_str(), key.c_str(), value);
}

float SettingsHandle::ReadValue(const stl::string& key, float defaultValue)
{
	return static_cast<float>(m_Settings.SimpleIni.GetDoubleValue(m_Section.c_str(), key.c_str(), defaultValue));
}
void SettingsHandle::WriteValue(const stl::string& key, float value)
{
	m_Settings.SimpleIni.SetDoubleValue(m_Section.c_str(), key.c_str(), value);
}

tmp::small_vector<tmp::string> SettingsHandle::ReadValue(const stl::string& key)
{
	CSimpleIniCaseA::TNamesDepend names;
	m_Settings.SimpleIni.GetAllValues(m_Section.c_str(), key.c_str(), names);
	tmp::small_vector<tmp::string> result;
	for (const auto& name : names)
	{
		result.push_back(name.pItem);
	}
	return result;
}

void SettingsHandle::DeleteValue(const stl::string& key)
{
	m_Settings.SimpleIni.Delete(m_Section.c_str(), key.c_str(), true);
}

SettingsManager::SettingsManager()
{
	m_Serializer.reset(new SettingsInternalParserHandleImpl());
	static_cast<SettingsInternalParserHandleImpl*>(m_Serializer.get())->SimpleIni.LoadFile("zmeyconfig.ini");
}
stl::shared_ptr<SettingsHandle> SettingsManager::DataFor(const stl::string& settingName)
{
	auto it = m_OpenSettingsMap.find(settingName);
	if (it == m_OpenSettingsMap.end())
	{
		m_OpenSettingsMap[settingName] = stl::make_shared<SettingsHandle>(*m_Serializer, settingName, *this);
	}
	return m_OpenSettingsMap[settingName];
}
void SettingsManager::OnSettingsClosed(const stl::string& settingsName)
{
	// TODO save the settings
}

}