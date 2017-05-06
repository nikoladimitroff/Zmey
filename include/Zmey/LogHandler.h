#pragma once

namespace Zmey
{
	enum class LogSeverity
	{
		Debug,
		Trace,
		Info,
		Warning,
		Error,
		Fatal
	};
	class ILogHandler
	{
	public:
		virtual ~ILogHandler() {}
		virtual void WriteLog(LogSeverity severity, const char* message) = 0;
	};
}