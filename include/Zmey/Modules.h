#pragma once

#include <Zmey/Tasks/TaskSystem.h>
#include <Zmey/Platform/Platform.h>

namespace Zmey
{
namespace Modules
{
// Global systems
extern Zmey::TaskSystem<4>* TaskSystem;
extern IPlatform* Platform;


void Initialize();
}
}