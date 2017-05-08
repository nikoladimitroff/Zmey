#include "Tasks.h"
#include <ctime>
namespace Zmey
{
namespace Chakra
{
ExecutionTask::ExecutionTask(JsValueRef func, int delay, JsValueRef thisArg, JsValueRef extraArgs, bool repeat)
{
	Func = func;
	Delay = delay;
	ArgCount = 1;
	Args[0] = thisArg;
	Args[1] = extraArgs;
	ShouldRepeat = repeat;
	Time = static_cast<int>(clock() / (double)(CLOCKS_PER_SEC / 1000));
	JsAddRef(Func, nullptr);
	JsAddRef(Args[0], nullptr);
	if (extraArgs != JS_INVALID_REFERENCE) {
		JsErrorCode err = JsAddRef(Args[1], nullptr);
		ArgCount = 2;
	}
}

JsValueRef ExecutionTask::Invoke()
{
	JsValueRef ret = JS_INVALID_REFERENCE;
	JsCallFunction(Func, Args, ArgCount, &ret);
	Time = static_cast<int>(clock() / (double)(CLOCKS_PER_SEC / 1000));
	return ret;
}

ExecutionTask::~ExecutionTask()
{
	JsRelease(Func, nullptr);
	JsRelease(Args[0], nullptr);
	if (Args[1] != JS_INVALID_REFERENCE) {
		JsRelease(Args[1], nullptr);
	}
}

}
}