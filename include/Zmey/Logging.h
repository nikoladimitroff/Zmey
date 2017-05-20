#pragma once
#include <string>
#include <stdio.h>
#include <Zmey/LogHandler.h>

namespace Zmey
{
extern Zmey::ILogHandler* GLogHandler;
}

#define LOG(Severity, Channel, Message) \
	do \
	{ \
		GLogHandler->WriteLog(Zmey::LogSeverity::##Severity, #Channel, Message); \
	} \
	while(0, 0)

#define FORMAT_LOG(Severity, Channel, Message, ...) \
	do \
	{ \
		auto scope = TempAllocator::GetTlsAllocator().ScopeNow(); \
		tmp::string buffer(sizeof(Message) * 5, '\0'); \
		sprintf_s(&buffer[0], sizeof(Message) * 5, Message, __VA_ARGS__); \
		GLogHandler->WriteLog(Zmey::LogSeverity::##Severity, #Channel, buffer.c_str()); \
	}\
	while(0, 0)


inline void ForceCrash()
{
	*((char*)-1) = 'x';
}

#define _STRINGIFY_MACRO(x) #x
#define STRINGIFY_MACRO(x) _STRINGIFY_MACRO(x)

#define ASSERT_RETURN(expression) \
	do \
	{ \
		if (!(expression)) \
		{ \
			LOG(Error, Assert, "Assert failed: " #expression " at " __FILE__ ":" STRINGIFY_MACRO(__LINE__)); \
			return; \
		} \
	} while(0, 0)

#define ASSERT_RETURN_VALUE(expression, defaultValue) \
	do \
	{ \
		if (!(expression)) \
		{ \
			LOG(Error, Assert, "Assert failed: " #expression " at " __FILE__ ":" STRINGIFY_MACRO(__LINE__)); \
			return defaultValue; \
		} \
	} while(0, 0)

#define ASSERT_FATAL(expression) \
	do \
	{ \
		if (!(expression)) \
		{ \
			LOG(Fatal, Assert, "Fatal assert failed: " #expression " at " __FILE__ ":" STRINGIFY_MACRO(__LINE__)); \
			ForceCrash(); \
		} \
	} while(0, 0)
