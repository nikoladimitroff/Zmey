#pragma once

namespace Zmey
{
namespace Chakra
{
// Used for repeatable execution
struct ExecutionTask
{
	ExecutionTask(JsValueRef func, int delay, JsValueRef thisArg, JsValueRef extraArgs, bool repeat = false);
	~ExecutionTask();
	ExecutionTask(const ExecutionTask&) = delete;
	ExecutionTask& operator=(const ExecutionTask&) = delete;

	JsValueRef Invoke();

	JsValueRef Func;
	JsValueRef Args[2];
	int ArgCount;
	int Delay;
	bool ShouldRepeat;
	int Time;
};

}
}