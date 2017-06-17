#pragma once
#include <Zmey/Scripting/Binding.h>
#include <Zmey/Modules.h>
#include <Zmey/Memory/MemoryManagement.h>

#include "ChakraScriptEngine.h"
namespace Zmey
{
namespace Chakra
{

namespace Binding
{

uint32_t GCurrentPendingClassProjectionIndex = 0u;
std::array<AutoNativeClassProjecter*, 256> GPendingClassProjections;

AutoNativeClassProjecter::AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype, uint16_t MemberCount,
	const wchar_t** memberNames, const JsNativeFunction* memberFuncs)
	: ClassName(className)
	, NameHash(0ull)
	, Constructor(constructor)
	, Prototype(prototype)
	, ActualMemberCount(MemberCount)
{
	std::memcpy(MemberNames, memberNames, MemberCount * sizeof(const wchar_t*));
	std::memcpy(MemberFuncs, memberFuncs, MemberCount * sizeof(JsNativeFunction));
	RegisterForInitialization();
}

AutoNativeClassProjecter::AutoNativeClassProjecter(const wchar_t* className, JsNativeFunction constructor, JsValueRef& prototype)
	: ClassName(className)
	, NameHash(0ull)
	, Constructor(constructor)
	, Prototype(prototype)
	, ActualMemberCount(0u)
{
	RegisterForInitialization();
}

void AutoNativeClassProjecter::RegisterForInitialization()
{
	JsContextRef activeContext;
	JsGetCurrentContext(&activeContext);
	if (!activeContext)
	{
		// We are still not initialized. Store this data for the initialize call
		GPendingClassProjections[GCurrentPendingClassProjectionIndex++] = this;
		return;
	}
	else
	{
		Project();
	}
}
void AutoNativeClassProjecter::Project()
{
	tmp::string utf8Name = Zmey::ConvertWideStringToUtf8(tmp::wstring(ClassName));
	NameHash = Zmey::Hash(Zmey::HashHelpers::CaseInsensitiveStringWrapper(utf8Name.c_str()));

	const tmp::vector<const wchar_t *> memberNames(MemberNames, MemberNames + ActualMemberCount);
	const tmp::vector<JsNativeFunction> memberFuncs(MemberFuncs, MemberFuncs + ActualMemberCount);
	ProjectNativeClass(ClassName, Constructor, Prototype, memberNames, memberFuncs);
}

JsValueRef AutoNativeClassProjecter::GetPrototypeOf(const Zmey::Hash classNameHash)
{
	for (uint32_t i = 0u; i < GCurrentPendingClassProjectionIndex; ++i)
	{
		auto projectionData = GPendingClassProjections[i];
		if (projectionData->NameHash == classNameHash)
		{
			return projectionData->Prototype;
		}
	}
	return nullptr;
}

void CheckChakraCall(JsErrorCode error, const char* functionCall, const char* file, int line)
{
	if (error != JsNoError)
	{
		FORMAT_LOG(Error, Scripting, "Chakra call %s failed at %s:%d with %d", functionCall, file, line, error);
	}
}
#define CHECKCHAKRA(Call) \
	CheckChakraCall(Call, #Call, __FILE__, __LINE__)

void Initialize()
{
	JsValueRef globalObject;
	CHECKCHAKRA(JsGetGlobalObject(&globalObject));
	JsValueRef console;
	CHECKCHAKRA(JsCreateObject(&console));
	SetProperty(globalObject, L"console", console);
	SetCallback(console, L"debug", JSConsoleDebug, nullptr);
	SetCallback(console, L"log", JSConsoleLog, nullptr);
	SetCallback(console, L"warn", JSConsoleWarn, nullptr);
	SetCallback(console, L"error", JSConsoleError, nullptr);
	SetCallback(globalObject, L"setTimeout", JSSetTimeout, nullptr);
	SetCallback(globalObject, L"setInterval", JSSetInterval, nullptr);

	for (uint32_t i = 0u; i < GCurrentPendingClassProjectionIndex; ++i)
	{
		auto projectionData = GPendingClassProjections[i];
		projectionData->Project();
	}
}
// The following funcs (SetCallback / SetProp / ProjectClass) were pretty much copied from Chakra's open gl sample
void SetCallback(JsValueRef object, const wchar_t *propertyName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	CHECKCHAKRA(JsGetPropertyIdFromName(propertyName, &propertyId));
	JsValueRef function;
	CHECKCHAKRA(JsCreateFunction(callback, callbackState, &function));
	CHECKCHAKRA(JsSetProperty(object, propertyId, function, true));
}

void SetProperty(JsValueRef object, const wchar_t *propertyName, JsValueRef property)
{
	JsPropertyIdRef propertyId;
	CHECKCHAKRA(JsGetPropertyIdFromName(propertyName, &propertyId));
	CHECKCHAKRA(JsSetProperty(object, propertyId, property, true));
}

void DefineProperty(JsValueRef object, const wchar_t* propertyName, JsNativeFunction getter)
{
	JsPropertyIdRef propertyId;
	CHECKCHAKRA(JsGetPropertyIdFromName(propertyName, &propertyId));
	JsValueRef propertyDescriptor;
	CHECKCHAKRA(JsCreateObject(&propertyDescriptor));
	JsValueRef getterFunc;
	CHECKCHAKRA(JsCreateFunction(getter, nullptr, &getterFunc));
	SetProperty(propertyDescriptor, L"get", getterFunc);
	bool ignoredResult;
	CHECKCHAKRA(JsDefineProperty(object, propertyId, propertyDescriptor, &ignoredResult));
}
void DefineProperty(JsValueRef object, const wchar_t* propertyName, JsNativeFunction getter, JsNativeFunction setter)
{
	JsPropertyIdRef propertyId;
	CHECKCHAKRA(JsGetPropertyIdFromName(propertyName, &propertyId));
	JsValueRef propertyDescriptor;
	CHECKCHAKRA(JsCreateObject(&propertyDescriptor));
	JsValueRef getterFunc;
	CHECKCHAKRA(JsCreateFunction(getter, nullptr, &getterFunc));
	SetProperty(propertyDescriptor, L"get", getterFunc);
	JsValueRef setterFunc;
	CHECKCHAKRA(JsCreateFunction(setter, nullptr, &setterFunc));
	SetProperty(propertyDescriptor, L"set", setterFunc);
	bool ignoredResult;
	CHECKCHAKRA(JsDefineProperty(object, propertyId, propertyDescriptor, &ignoredResult));
}

void ProjectNativeClass(const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, const tmp::vector<const wchar_t *>& memberNames, const tmp::vector<JsNativeFunction>& memberFuncs)
{
	// create class's prototype and project its member functions
	JsCreateObject(&prototype);
	assert(memberNames.size() == memberNames.size());
	for (int i = 0; i < memberNames.size(); ++i) {
		SetCallback(prototype, memberNames[i], memberFuncs[i], nullptr);
	}
	if (constructor)
	{
		JsValueRef globalObject;
		CHECKCHAKRA(JsGetGlobalObject(&globalObject));
		JsValueRef jsConstructor;
		CHECKCHAKRA(JsCreateFunction(constructor, nullptr, &jsConstructor));
		SetProperty(globalObject, className, jsConstructor);
		SetProperty(jsConstructor, L"prototype", prototype);
	}
}

void ProjectGlobal(const wchar_t* globalName, void* objectToProject, Zmey::Hash classNameHash)
{
	JsValueRef globalObject;
	CHECKCHAKRA(JsGetGlobalObject(&globalObject));
	JsValueRef object;
	CHECKCHAKRA(JsCreateExternalObject(objectToProject, nullptr, &object));
	SetProperty(globalObject, globalName, object);
	JsValueRef prototype = AutoNativeClassProjecter::GetPrototypeOf(classNameHash);
	CHECKCHAKRA(JsSetPrototype(object, prototype));
}

uint32_t GCurrentAnyTypeIndex = 0u;
std::array<stl::unique_ptr<AnyTypeData>, 256> GAnyTypeList;

void RegisterPrototypesForAnyTypeSet(AnyTypeData data)
{
	GAnyTypeList[GCurrentAnyTypeIndex] = stl::make_unique<AnyTypeData>(data);

	JsValueRef globalObject;
	CHECKCHAKRA(JsGetGlobalObject(&globalObject));
	JsValueRef enumObject;
	CHECKCHAKRA(JsCreateObject(&enumObject));
	auto scope = Zmey::TempAllocator::GetTlsAllocator().ScopeNow();
	for (int i = 0; i < data.PrototypeNames.size(); ++i)
	{
		JsValueRef indexValue;
		CHECKCHAKRA(JsIntToNumber(i, &indexValue));
		tmp::wstring name = Zmey::ConvertUtf8ToWideString(tmp::string(data.PrototypeNames[i]));
		SetProperty(enumObject, name.c_str(), indexValue);
	}
	tmp::wstring anySetName = Zmey::ConvertUtf8ToWideString(tmp::string(data.Name));
	SetProperty(globalObject, anySetName.c_str(), enumObject);
}
JsValueRef GetProtototypeOfAnyTypeSet(Zmey::Hash anyTypeName, int index)
{
	auto it = std::find_if(GAnyTypeList.begin(), GAnyTypeList.begin() + GCurrentAnyTypeIndex, [anyTypeName](const stl::unique_ptr<AnyTypeData>& data)
	{
		return data->NameHash == anyTypeName;
	});
	ASSERT_FATAL(it != GAnyTypeList.end());
	return (*it)->Prototypes[index];
}

JsValueRef CALLBACK JSConsoleDebug(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void *callbackState)
{
	return JSConsoleMessage(Zmey::LogSeverity::Debug, callee, isConstructCall, arguments, argumentCount, callbackState);
}
JsValueRef CALLBACK JSConsoleLog(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void *callbackState)
{
	return JSConsoleMessage(Zmey::LogSeverity::Info, callee, isConstructCall, arguments, argumentCount, callbackState);
}
JsValueRef CALLBACK JSConsoleWarn(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void *callbackState)
{
	return JSConsoleMessage(Zmey::LogSeverity::Warning, callee, isConstructCall, arguments, argumentCount, callbackState);
}
JsValueRef CALLBACK JSConsoleError(JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void *callbackState)
{
	return JSConsoleMessage(Zmey::LogSeverity::Error, callee, isConstructCall, arguments, argumentCount, callbackState);
}

JsValueRef JSConsoleMessage(Zmey::LogSeverity severity, JsValueRef callee, bool isConstructCall, JsValueRef* arguments, unsigned short argumentCount, void *callbackState)
{
	auto tempScope = TempAllocator::GetTlsAllocator().ScopeNow();
	tmp::wstring message;
	for (unsigned int index = 1; index < argumentCount; index++)
	{
		if (index > 1)
		{
			message += ' ';
		}
		JsValueRef stringValue;
		CHECKCHAKRA(JsConvertValueToString(arguments[index], &stringValue));
		const wchar_t* string;
		size_t length;
		CHECKCHAKRA(JsStringToPointer(stringValue, &string, &length));
		message.append(string);
	}

	tmp::string mbMessage = Zmey::ConvertWideStringToUtf8(message);
	Zmey::GLogHandler->WriteLog(severity, "Script", mbMessage.c_str());
	return JS_INVALID_REFERENCE;
}

JsValueRef CALLBACK Binding::JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 3);
	JsValueRef func = arguments[1];
	int delay = 0;
	CHECKCHAKRA(JsNumberToInt(arguments[2], &delay));
	auto host = static_cast<ChakraScriptEngine*>(Zmey::Modules::ScriptEngine);
	host->m_ExecutionTasks.push_back(stl::make_unique<ExecutionTask>(func, delay, arguments[0], JS_INVALID_REFERENCE));
	return JS_INVALID_REFERENCE;
}

// JsNativeFunction for setInterval(func, delay)
JsValueRef CALLBACK Binding::JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 3);
	JsValueRef func = arguments[1];
	int delay = 0;
	CHECKCHAKRA(JsNumberToInt(arguments[2], &delay));
	auto host = static_cast<ChakraScriptEngine*>(Zmey::Modules::ScriptEngine);
	host->m_ExecutionTasks.push_back(stl::make_unique<ExecutionTask>(func, delay, arguments[0], JS_INVALID_REFERENCE, true));
	return JS_INVALID_REFERENCE;
}
}
}
}