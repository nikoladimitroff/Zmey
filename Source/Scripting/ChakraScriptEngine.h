#include <Zmey/ScriptEngine.h>

#include <atomic>
#include <thread>

#include <ChakraCore/ChakraCore.h>

#include <Zmey/Tasks/BlockingQueue.h>
#include "Tasks.h"
#include "Binding.h"

namespace Zmey
{
namespace Chakra
{
class ChakraScriptEngine : public IScriptEngine
{
public:
	ChakraScriptEngine();
	~ChakraScriptEngine();

	virtual void ExecuteFromFile(ResourceId) override;
	virtual void ExecuteNextFrame(float deltams) override;
private:
	void Run();
	void RunOneLoopIteration();
	friend JsValueRef CALLBACK Zmey::Chakra::Binding::JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
	friend JsValueRef CALLBACK Zmey::Chakra::Binding::JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

	std::thread m_ScriptingThread;
	std::atomic_bool m_KeepRunning;
	Zmey::BlockingQueue<ScriptTask> m_ScriptTasks;
	Zmey::BlockingQueue<FrameTask> m_FrameTasks;
	// No need for thread-safety; this queue is only accessed on the scripting thread
	stl::deque<stl::unique_ptr<ExecutionTask>> m_ExecutionTasks;

	JsRuntimeHandle m_Runtime;
	JsContextRef m_Context;
	unsigned m_CurrentSourceContext;
};

}
}