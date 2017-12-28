#include <algorithm>
#include <memory>

#include <Zmey/EngineLoop.h>
#include <GiftOfTheSanctumGame.h>



int main()
{
	GiftOfTheSanctumGame game;
	Zmey::EngineLoop loop(&game);
	loop.Run();
	return 0;
}
