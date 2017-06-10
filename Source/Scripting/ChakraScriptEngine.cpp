#include "ChakraScriptEngine.h"

#include <ctime>

#include <Zmey/Logging.h>
#include <Zmey/Modules.h>
#include <Zmey/Scripting/Binding.h>

namespace Zmey
{
namespace Chakra
{

ChakraScriptEngine::ChakraScriptEngine()
	: m_CurrentSourceContext(0u)
{
	// Initialize chakra
	JsRuntimeAttributes attributes = static_cast<JsRuntimeAttributes>(
		JsRuntimeAttributeDisableBackgroundWork |
		JsRuntimeAttributeEnableIdleProcessing);
	ASSERT_FATAL(JsCreateRuntime(attributes, nullptr, &m_Runtime) == JsNoError);
	ASSERT_FATAL(JsCreateContext(m_Runtime, &m_Context) == JsNoError);
	ASSERT_FATAL(JsSetCurrentContext(m_Context) == JsNoError);

	Binding::Initialize();
}
ChakraScriptEngine::~ChakraScriptEngine()
{
	JsSetCurrentContext(JS_INVALID_REFERENCE);
	JsDisposeRuntime(m_Runtime);
}

void ChakraScriptEngine::ExecuteNextFrame(float deltaTime)
{
	// Execution order is as follows:
	// 1. If there's nothing to do, call JSIdle
	// 2. Execute pending script requests
	// 3. Execute frames
	// 4. Execute pending time tasks

	auto totalTaskCount = m_ScriptTasks.size() + m_ExecutionTasks.size();
	if (totalTaskCount == 0)
	{
		JsIdle(nullptr);
	}
	while (m_ScriptTasks.size())
	{
		auto scriptTask = std::move(m_ScriptTasks.front());
		auto source = scriptTask.Script.c_str();
		ExecuteScript(source, L"");
		m_ScriptTasks.pop_front();
	}
	wchar_t buffer[32];
	wsprintfW(buffer, L"nextFrame(%d);", deltaTime / 1000.f);
	ExecuteScript(buffer, L"frameupdate");

	auto executionTasksCount = m_ExecutionTasks.size();
	for (unsigned i = 0; i < executionTasksCount; ++i)
	{
		stl::unique_ptr<ExecutionTask> task = std::move(m_ExecutionTasks.front());
		m_ExecutionTasks.pop_front();

		int currentTime = static_cast<int>(clock() / (double)(CLOCKS_PER_SEC / 1000));
		if (currentTime - task->Time > task->Delay)
		{
			task->Invoke();
			if (task->ShouldRepeat)
			{
				m_ExecutionTasks.push_back(std::move(task));
			}
		}
		else
		{
			m_ExecutionTasks.push_back(std::move(task));
		}
	}
}

void ChakraScriptEngine::ExecuteFromFile(Zmey::Name name)
{
	auto scriptSource = Zmey::Modules::ResourceLoader->AsText(name);
	m_ScriptTasks.push_back(std::move(ScriptTask(*scriptSource)));
}

tmp::string StringifyJsValue(JsValueRef value)
{
	JsPropertyIdRef toStringFuncId;
	::JsGetPropertyIdFromName(L"toString", &toStringFuncId);
	JsValueRef toStringFunc;
	::JsGetProperty(value, toStringFuncId, &toStringFunc);
	JsValueRef stringResult;
	::JsCallFunction(toStringFunc, &value, 1, &stringResult);
	const wchar_t* stringPtr;
	size_t stringLength;
	::JsStringToPointer(stringResult, &stringPtr, &stringLength);
	tmp::wstring wideText(stringPtr, stringLength);
	tmp::string utf8text = Zmey::ConvertWideStringToUtf8(wideText);
	return utf8text;
}

void ChakraScriptEngine::ExecuteScript(const wchar_t* script, const wchar_t* scriptSourceUrl)
{
	JsErrorCode error = JsRunScript(script, m_CurrentSourceContext++, scriptSourceUrl, nullptr);
	if (error != JsNoError)
	{
		if (error == JsErrorScriptException || error == JsErrorScriptCompile)
		{
			JsValueRef exception;
			JsGetAndClearException(&exception);
			FORMAT_LOG(Error, Script, "Exception thrown: %s", StringifyJsValue(exception).c_str());
		}
		else
		{
			FORMAT_LOG(Error, Script, "Script execution resulted in error: %d", error);
		}
	}
}

}
}
