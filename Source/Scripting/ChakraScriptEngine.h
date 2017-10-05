#include <Zmey/Scripting/ScriptEngine.h>

#include <atomic>
#include <thread>

#include <ChakraCore/ChakraCore.h>

#include <Zmey/Scripting/Binding.h>
#include "Tasks.h"

namespace Zmey
{
namespace Chakra
{
class ChakraScriptEngine : public IScriptEngine
{
public:
	ChakraScriptEngine();
	~ChakraScriptEngine();

	virtual void ExecuteFromFile(Zmey::Name) override;
	virtual void ExecuteNextFrame(float deltaTime) override;
private:
	void ExecuteScript(const wchar_t* script, const wchar_t* scriptSourceUrl);
	friend JsValueRef CALLBACK Zmey::Chakra::Binding::JSSetTimeout(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);
	friend JsValueRef CALLBACK Zmey::Chakra::Binding::JSSetInterval(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState);

	stl::deque<ScriptTask> m_ScriptTasks;
	// No need for thread-safety; this queue is only accessed on the scripting thread
	stl::deque<stl::unique_ptr<ExecutionTask>> m_ExecutionTasks;

	JsRuntimeHandle m_Runtime;
	JsContextRef m_Context;
	unsigned m_CurrentSourceContext;
};

}
}
