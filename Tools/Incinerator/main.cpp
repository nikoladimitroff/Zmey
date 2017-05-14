#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <nlohmann/json.hpp>

#include <Zmey/Components/ComponentRegistry.h>

struct Options
{
	std::string GameDirectory;
};

void Incinerate(const Options& options)
{
}

int main(int argc, char** argv)
{
	Options options;
	for (int i = 1; i < argc; ++i)
	{
		if (std::strcmp(argv[i], "--game") == 0)
		{
			options.GameDirectory = argv[++i];
		}
	}
	Incinerate(options);

	return 0;
}