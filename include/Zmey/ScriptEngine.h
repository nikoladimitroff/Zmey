#pragma once
#include <Zmey/Memory/MemoryManagement.h>

namespace Zmey
{

class IScriptEngine
{
public:
	virtual ~IScriptEngine() {}
	virtual void ExecuteFromFile(const stl::string& filePath) = 0;
	virtual void ExecuteNextFrame() = 0;
};

}