#pragma once

#include <Zmey/Graphics/Features/MeshRenderer.h>
#include <Zmey/Graphics/Features/RectRenderer.h>

// (Name of the namespace, Has GatherData, Has PrepareData, Has GenerateCommands)
#define RENDER_FEATURE_MACRO_ITERATOR(MACRO) \
	MACRO(MeshRenderer, true, true, true) \
	MACRO(RectRenderer, true, false, true) \

