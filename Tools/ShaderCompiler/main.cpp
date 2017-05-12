#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static const std::string sSourceFolder = "Source/Graphics/Shaders/Source/";
static const std::string sOutputFolder = "Source/Graphics/Shaders/Compiled/";
static const std::string sFileWildcard = "*.hlsl";
static const std::string sCompiler = "glslc.exe";

#define PRINT_CMD_LINE 0
#define FORCE_REBUILD 1

int main()
{
	std::string shadersFolder = sSourceFolder;
	shadersFolder.append(sFileWildcard);

	std::vector<std::string> fileList;
	WIN32_FIND_DATA ffd;
	auto hFind = FindFirstFile(shadersFolder.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind)
	{
		std::cerr << "Failed to find shaders to compile!" << std::endl;
		return -1;
	}

	do
	{
		std::string filename(ffd.cFileName);
		filename = filename.substr(0, filename.find_last_of('.'));
		fileList.push_back(filename);
	} while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	std::cout << "Found " << fileList.size() << " files." << std::endl;

	for (auto& shader : fileList) {
		const auto filename = sSourceFolder + shader + ".hlsl";
		const auto finalname = sOutputFolder + shader;

		const auto entryName = shader.substr(3);

		std::cout << "Compiling " << shader << "..." << std::endl;

		std::string profile;

		std::ostringstream defines;

		std::ostringstream compilCmd;
		compilCmd << sCompiler
			<< " -Werror "
			<< "-x hlsl "
			<< "-mfmt=num " // print hex numbers
			<< "--target-env=vulkan "
			<< "-I " + sSourceFolder + "include "
			<< defines.str();

		std::ostringstream compilCmdVS;
		compilCmdVS << compilCmd.str()
			<< "-o " << finalname + "VS.h "
			<< "-fshader-stage=vertex "
			<< "-fentry-point=VertexShaderMain "
			<< filename;

		std::ostringstream compilCmdPS;
		compilCmdPS << compilCmd.str()
			<< "-o " << finalname + "PS.h "
			<< "-fshader-stage=fragment "
			<< "-fentry-point=PixelShaderMain "
			<< filename;

#if PRINT_CMD_LINE
		std::cout << compilCmdVS.str() << std::endl;
		std::cout << compilCmdPS.str() << std::endl;
#endif

		{
			const auto compilRes = system(compilCmdVS.str().c_str());
			if (compilRes != 0)
			{
				std::cerr << "Compilation error on shader vertex stage: " << shader << std::endl;
				system("pause");
				exit(1);
			}
		}

		{
			const auto compilRes = system(compilCmdPS.str().c_str());
			if (compilRes != 0)
			{
				std::cerr << "Compilation error on shader pixel stage: " << shader << std::endl;
				system("pause");
				exit(1);
			}
		}
	}
	return 0;
}