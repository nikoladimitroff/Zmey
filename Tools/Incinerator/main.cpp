#include <string>

#include <Zmey/EngineLoop.h>

#include "Incinerator.h"


int main(int argc, char** argv)
{
	Incinerator::Options options;
	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--game") == 0)
		{
			options.GameDirectory = argv[++i];
		}
	}
	Zmey::EngineLoop loop(nullptr); // Neccessary to initialize the engine TODO: change the api to something not as ugly
	Incinerator incinerator;
	incinerator.Incinerate(options);

	return 0;
}