#pragma once

#include <Zmey/Tasks/TaskSystem.h>

namespace Zmey
{
	namespace Modules
	{
		extern Zmey::TaskSystem<4>* TaskSystem;
		void Initialize();
	}
}