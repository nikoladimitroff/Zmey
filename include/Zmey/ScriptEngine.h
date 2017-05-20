#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{

class IScriptEngine
{
public:
	virtual ~IScriptEngine() {}
	virtual void ExecuteFromFile(ResourceId id) = 0;
	virtual void ExecuteNextFrame(float deltaTime) = 0;
};

}