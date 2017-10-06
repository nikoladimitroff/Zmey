#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>

// (Name of the namespace, Has GatherData, Has PrepareData, Has GenerateCommands)
#define RENDER_FEATURE_MACRO_ITERATOR(MACRO) \
	MACRO(MeshRenderer, true, true, true) \

