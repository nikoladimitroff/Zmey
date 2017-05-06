#include <Zmey/Modules.h>

namespace Zmey
{
	namespace Modules
	{
		Zmey::TaskSystem<4>* TaskSystem;

		void Initialize()
		{
			TaskSystem = StaticAlloc<Zmey::TaskSystem<4>>();
		}
	}
}