#pragma once

#include <Zmey/Config.h>

#define BUILD_WITH_EASY_PROFILER
#include <easy/profiler.h>


#define PROFILE_INITIALIZE profiler::startListen()
#define PROFILE_DESTROY profiler::stopListen()


#define PROFILE_SCOPE EASY_BLOCK(__FUNCTION__)

