#pragma once
#include <ChakraCore/ChakraCore.h>
#include <Zmey/Hash.h>
#include <Zmey/LogHandler.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
inline tmp::string ConvertWideStringToUtf8(const tmp::wstring& wString)
{
	tmp::string utf8String;
	auto utf8Size = wcstombs(nullptr, wString.c_str(), wString.size());
	utf8String.assign(utf8Size, ' ');
	wcstombs(&utf8String[0], wString.c_str(), wString.size());
	return utf8String;
}
inline tmp::wstring ConvertUtf8ToWideString(const tmp::string& utf8String)
{
	tmp::wstring wideString;
	auto wideSize = mbstowcs(nullptr, utf8String.c_str(), utf8String.size());
	wideString.assign(wideSize, L' ');
	mbstowcs(&wideString[0], utf8String.c_str(), utf8String.size());
	return wideString;
}

inline stl::string ConvertWideStringToUtf8(const stl::wstring& wString)
{
	stl::string utf8String;
	auto utf8Size = wcstombs(nullptr, wString.c_str(), wString.size());
	utf8String.assign(utf8Size, ' ');
	wcstombs(&utf8String[0], wString.c_str(), wString.size());
	return utf8String;
}
inline stl::wstring ConvertUtf8ToWideString(const stl::string& utf8String)
{
	stl::wstring wideString;
	auto wideSize = mbstowcs(nullptr, utf8String.c_str(), utf8String.size());
	wideString.assign(wideSize, L' ');
	mbstowcs(&wideString[0], utf8String.c_str(), utf8String.size());
	return wideString;
}
namespace Chakra
{

namespace Binding
{
struct AutoNativeClassProjecter
{
	static constexpr uint16_t MaxMemberCount = 16u;
	AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype, uint16_t MemberCount,
		const wchar_t** memberNames, const JsNativeFunction* memberFuncs);

	AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype);

	static JsValueRef GetPrototypeOf(const Zmey::Hash classNameHash);
private:
	friend void Initialize();
	void RegisterForInitialization();
	void Project();

	const wchar_t* ClassName;
	Zmey::Hash NameHash;
	const JsNativeFunction Constructor;
	JsValueRef& Prototype;
	uint16_t ActualMemberCount;
	const wchar_t* MemberNames[MaxMemberCount];
	JsNativeFunction MemberFuncs[MaxMemberCount];
};
void Initialize();

void SetCallback(JsValueRef object, const wchar_t* propertyName, JsNativeFunction callback, void *callbackState);
void SetProperty(JsValueRef object, const wchar_t* propertyName, JsValueRef property);
void DefineProperty(JsValueRef object, const wchar_t* propertyName, JsNativeFunction getter);
void DefineProperty(JsValueRef object, const wchar_t* propertyName, JsNativeFunction getter, JsNativeFunction setter);
void ProjectNativeClass(const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, const tmp::vector<const wchar_t *>& memberNames, const tmp::vector<JsNativeFunction>& memberFuncs);
void ProjectGlobal(const wchar_t* globalName, void* objectToProject, Zmey::Hash classNameHash);
template<typename T>
void FinalizeCallback(void* data)
{
	delete reinterpret_cast<T*>(data);
}

struct AnyTypeData
{
	AnyTypeData(const char* name, const stl::small_vector<const char*>& names, const stl::small_vector<JsValueRef>& prototypes)
		: Name(name)
		, NameHash(name)
		, PrototypeNames(names)
		, Prototypes(prototypes)
	{}
	const char* Name;
	Zmey::Hash NameHash;
	stl::small_vector<const char*> PrototypeNames;
	stl::small_vector<JsValueRef> Prototypes;
};
void RegisterPrototypesForAnyTypeSet(AnyTypeData data);
JsValueRef GetProtototypeOfAnyTypeSet(Zmey::Hash anyTypeName, int index);

JsValueRef CALLBACK JSConsoleDebug(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleLog(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleWarn(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleError(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleMessage(Zmey::LogSeverity severity, JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

JsValueRef CALLBACK JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);


void CheckChakraCall(JsErrorCode error, const char* functionCall, const char* file, int line);
#define CHECKCHAKRA(Call) \
	Zmey::Chakra::Binding::CheckChakraCall(Call, #Call, __FILE__, __LINE__)
}
}
}