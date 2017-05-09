#pragma once
#include "Binding.h"
#include <Zmey/Modules.h>

#include "ChakraScriptEngine.h"
namespace Zmey
{
namespace Chakra
{

namespace Binding
{

void Initialize()
{
	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);
	JsValueRef console;
	JsCreateObject(&console);
	SetProperty(globalObject, L"console", console);
	SetCallback(console, L"debug", JSConsoleDebug, nullptr);
	SetCallback(console, L"log", JSConsoleLog, nullptr);
	SetCallback(console, L"warn", JSConsoleWarn, nullptr);
	SetCallback(console, L"error", JSConsoleError, nullptr);
	SetCallback(globalObject, L"setTimeout", JSSetTimeout, nullptr);
	SetCallback(globalObject, L"setInterval", JSSetInterval, nullptr);
}
// The following funcs (SetCallback / SetProp / ProjectClass) were pretty much copied from Chakra's open gl sample
void SetCallback(JsValueRef object, const wchar_t *propertyName, JsNativeFunction callback, void *callbackState)
{
	JsPropertyIdRef propertyId;
	JsGetPropertyIdFromName(propertyName, &propertyId);
	JsValueRef function;
	JsCreateFunction(callback, callbackState, &function);
	JsSetProperty(object, propertyId, function, true);
}

void SetProperty(JsValueRef object, const wchar_t *propertyName, JsValueRef property)
{
	JsPropertyIdRef propertyId;
	JsGetPropertyIdFromName(propertyName, &propertyId);
	JsSetProperty(object, propertyId, property, true);
}

void ProjectNativeClass(const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, tmp::vector<const wchar_t *> memberNames, tmp::vector<JsNativeFunction> memberFuncs)
{
	JsValueRef globalObject;
	JsGetGlobalObject(&globalObject);
	JsValueRef jsConstructor;
	JsCreateFunction(constructor, nullptr, &jsConstructor);
	SetProperty(globalObject, className, jsConstructor);
	// create class's prototype and project its member functions
	JsCreateObject(&prototype);
	assert(memberNames.size() == memberNames.size());
	for (int i = 0; i < memberNames.size(); ++i) {
		SetCallback(prototype, memberNames[i], memberFuncs[i], nullptr);
	}
	SetProperty(jsConstructor, L"prototype", prototype);
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
		JsConvertValueToString(arguments[index], &stringValue);
		const wchar_t* string;
		size_t length;
		JsStringToPointer(stringValue, &string, &length);
		message.append(string);
	}

	tmp::string mbMessage;
	auto mbSize = wcstombs(nullptr, message.c_str(), message.size());
	mbMessage.assign(mbSize, L' ');
	wcstombs(&mbMessage[0], message.c_str(), message.size());
	Zmey::GLogHandler->WriteLog(severity, "Script", mbMessage.c_str());
	return JS_INVALID_REFERENCE;
}

JsValueRef CALLBACK Binding::JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
	assert(!isConstructCall && argumentCount == 3);
	JsValueRef func = arguments[1];
	int delay = 0;
	JsNumberToInt(arguments[2], &delay);
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
	JsNumberToInt(arguments[2], &delay);
	auto host = static_cast<ChakraScriptEngine*>(Zmey::Modules::ScriptEngine);
	host->m_ExecutionTasks.push_back(stl::make_unique<ExecutionTask>(func, delay, arguments[0], JS_INVALID_REFERENCE, true));
	return JS_INVALID_REFERENCE;
}
}
}
}