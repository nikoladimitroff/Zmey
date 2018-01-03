#include <algorithm>
#include <memory>

#include <Zmey/EngineLoop.h>
#include <Zmey/Modules.h>
#include <GiftOfTheSanctumGame.h>



int main()
{
	GiftOfTheSanctumGame game;
	Zmey::EngineLoop loop(&game);
	loop.Run();
	Zmey::Modules.Uninitialize();
	return 0;
}
