#include "ChakraScriptEngine.h"

#include <ctime>

#include <Zmey/Logging.h>
#include <Zmey/Modules.h>

#include "Binding.h"

namespace Zmey
{
namespace Chakra
{

ChakraScriptEngine::ChakraScriptEngine()
	: m_ScriptingThread(&ChakraScriptEngine::Run, this)
	, m_KeepRunning(true)
	, m_CurrentSourceContext(0u)
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

	Binding::Initialize();
	while (m_KeepRunning)
	{
		RunOneLoopIteration();
	}
	JsSetCurrentContext(JS_INVALID_REFERENCE);
	JsDisposeRuntime(m_Runtime);
}

void ChakraScriptEngine::RunOneLoopIteration()
{
	// Execution order is as follows:
	// 1. If there's nothing to do, call JSIdle
	// 2. Execute pending script requests
	// 3. Execute pending frames
	// 4. Execute pending time tasks

	// Store the sizes of all 3 queues so that we don't execute tasks that have been added after the iteration has begun
	unsigned scriptTasksCount = m_ScriptTasks.Size();
	unsigned frameTaskCount = m_FrameTasks.Size();
	unsigned executionTasksCount = static_cast<unsigned>(m_ExecutionTasks.size());
	auto totalTaskCount = scriptTasksCount + frameTaskCount + executionTasksCount;
	if (totalTaskCount == 0)
	{
		JsIdle(nullptr);
	}
	for (unsigned i = 0u; i < scriptTasksCount; ++i)
	{
		auto scriptTask = std::move(m_ScriptTasks.Dequeue());
		auto source = scriptTask.Script.c_str();
		JsErrorCode error = JsRunScript(source, m_CurrentSourceContext++, L"", nullptr);
		if (error != JsNoError)
		{
			FORMAT_LOG(Error, Script, "Script execution resulted in error: %d", error);
		}
	}
	for (unsigned i = 0; i < frameTaskCount; ++i)
	{
		auto frameTask = m_FrameTasks.Dequeue();
		wchar_t buffer[32];
		wsprintfW(buffer, L"nextFrame(%d);", frameTask.DeltaMs);
		JsRunScript(buffer, m_CurrentSourceContext++, L"", nullptr);
	}
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

void ChakraScriptEngine::ExecuteFromFile(ResourceId id)
{
	auto scriptSource = Zmey::Modules::ResourceLoader->As<char>(id);
	m_ScriptTasks.Enqueue(std::move(ScriptTask(scriptSource)));
}

void ChakraScriptEngine::ExecuteNextFrame(float deltaTime)
{
	m_FrameTasks.Enqueue(std::move(FrameTask(deltaTime)));
}

}
}
