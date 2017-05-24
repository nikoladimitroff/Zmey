#pragma once
#include <ChakraCore/ChakraCore.h>
#include <Zmey/LogHandler.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
tmp::string ConvertWideStringToUtf8(const tmp::wstring& wString)
{
	tmp::string mbString;
	auto mbSize = wcstombs(nullptr, wString.c_str(), wString.size());
	mbString.assign(mbSize, L' ');
	wcstombs(&mbString[0], wString.c_str(), wString.size());
	return mbString;
}
namespace Chakra
{

namespace Binding
{
struct AutoNativeClassProjecter
{
	static constexpr uint16_t MaxMemberCount = 16u;
	template<uint16_t MemberCount>
	AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype,
		const wchar_t* memberNames[MemberCount], const JsNativeFunction memberFuncs[MemberCount])
		: ClassName(className)
		, Constructor(constructor)
		, Prototype(prototype)
		, ActualMemberCount(MemberCount)
	{
		std::memcpy(MemberNames, memberNames, sizeof(memberNames));
		std::memcpy(MemberNames, memberNames, sizeof(memberNames));
		RegisterForInitialization();
	}

	AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype)
		: ClassName(className)
		, Constructor(constructor)
		, Prototype(prototype)
		, ActualMemberCount(0u)
	{
		RegisterForInitialization();
	}

private:
	friend void Initialize();
	void RegisterForInitialization();
	void Project();

	const wchar_t* ClassName;
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

JsValueRef CALLBACK JSConsoleDebug(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleLog(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleWarn(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleError(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSConsoleMessage(Zmey::LogSeverity severity, JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

JsValueRef CALLBACK JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
JsValueRef CALLBACK JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

}
}
}