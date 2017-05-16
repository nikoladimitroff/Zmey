#include <string>

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
	Incinerator incinerator;
	incinerator.Incinerate(options);

	return 0;
}