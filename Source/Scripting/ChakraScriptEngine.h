#include <Zmey/ScriptEngine.h>

#include <atomic>
#include <thread>

#include <ChakraCore/ChakraCore.h>

#include "ExecutionTask.h"

namespace Zmey
{
namespace Chakra
{
class ChakraScriptEngine : public IScriptEngine
{
public:
	ChakraScriptEngine();
	~ChakraScriptEngine();

	virtual void ExecuteFromFile(const stl::string& filePath) override;
	virtual void ExecuteNextFrame() override;
private:
	void Run();
	std::thread m_ScriptingThread;
	std::atomic_bool m_KeepRunning;
	std::atomic_uint8_t m_PendingFrameCounter;
	stl::deque<ExecutionTask> m_PendingTasks;
	JsRuntimeHandle m_Runtime;
	JsContextRef m_Context;
};

}
}
