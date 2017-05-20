#pragma once
#include <ChakraCore/ChakraCore.h>

#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{
namespace Chakra
{
// Used for repeatable execution of scripts; e.g. from setTimeout / setInterval
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

struct ScriptTask
{
	ScriptTask(const stl::string& script)
		: Script(script.size(), L' ')
	{
		mbstowcs(const_cast<wchar_t*>(&Script[0]), script.c_str(), Script.size());
	}
	const stl::wstring Script;
};

struct FrameTask
{
	FrameTask(float delta)
		: DeltaMs(delta / 1000.f)
	{}
	float DeltaMs;
};

}
}