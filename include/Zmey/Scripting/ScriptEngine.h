#pragma once
#include <Zmey/Memory/MemoryManagement.h>
#include <Zmey/ResourceLoader/ResourceLoader.h>

namespace Zmey
{

class ZMEY_API IScriptEngine
{
public:
	virtual ~IScriptEngine() {}
	virtual void ExecuteFromFile(Zmey::Name name) = 0;
	virtual void ExecuteNextFrame(float deltaTime) = 0;
};

}