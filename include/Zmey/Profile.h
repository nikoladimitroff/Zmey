#pragma once

#include <Zmey/Config.h>

#define BUILD_WITH_EASY_PROFILER
#include <easy/profiler.h>

#define PROFILE_INITIALIZE profiler::startListen(); EASY_PROFILER_ENABLE;
#define PROFILE_DESTROY profiler::stopListen()

#define PROFILE_SET_THREAD_NAME(name) EASY_THREAD(name)


#define PROFILE_FUNCTION EASY_FUNCTION
#define PROFILE_SCOPE(name) EASY_BLOCK(name)

#define PROFILE_START_BLOCK(name) EASY_NONSCOPED_BLOCK(name)
#define PROFILE_END_BLOCK EASY_END_BLOCK

