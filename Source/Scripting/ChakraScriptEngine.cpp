#include "ChakraScriptEngine.h"
#include <Zmey/Logging.h>

namespace Zmey
{
namespace Chakra
{

ChakraScriptEngine::ChakraScriptEngine()
	: m_ScriptingThread(&ChakraScriptEngine::Run, this)
	, m_KeepRunning(true)
	, m_PendingFrameCounter(0)
{
}
ChakraScriptEngine::~ChakraScriptEngine()
{
	m_ScriptingThread.join();
}

void ChakraScriptEngine::Run()
{
	// Initialize chakra
	JsRuntimeAttributes attributes = static_cast<JsRuntimeAttributes>(
		JsRuntimeAttributeDisableBackgroundWork |
		JsRuntimeAttributeEnableIdleProcessing);
	ASSERT_FATAL(JsCreateRuntime(attributes, nullptr, &m_Runtime) == JsNoError);
	ASSERT_FATAL(JsCreateContext(m_Runtime, &m_Context) == JsNoError);
	ASSERT_FATAL(JsSetCurrentContext(m_Context) == JsNoError);

	while (m_KeepRunning)
	{
		// Execution order is as follows:
		// 1. If there's nothing to do, call JSIdle
		// 2. Execute pending script requests
		// 3. Execute pending frames
		// 4. Execute pending time tasks
	}
	JsSetCurrentContext(JS_INVALID_REFERENCE);
	JsDisposeRuntime(m_Runtime);
}

void ChakraScriptEngine::ExecuteFromFile(const stl::string& filePath)
{

}

}
}
