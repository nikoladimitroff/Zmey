#pragma once
#include <ChakraCore/ChakraCore.h>
#include <Zmey/LogHandler.h>
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Chakra
{

namespace Binding
{
void Initialize();

void SetCallback(JsValueRef object, const wchar_t *propertyName, JsNativeFunction callback, void *callbackState);
void SetProperty(JsValueRef object, const wchar_t *propertyName, JsValueRef property);
void ProjectNativeClass(const wchar_t *className, JsNativeFunction constructor, JsValueRef &prototype, tmp::vector<const wchar_t *> memberNames, tmp::vector<JsNativeFunction> memberFuncs);

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