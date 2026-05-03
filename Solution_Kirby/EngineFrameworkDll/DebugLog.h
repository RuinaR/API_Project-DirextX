#pragma once

#include "EngineFrameworkAPI.h"
#include <string>
#include <vector>

enum class DebugLogLevel
{
	Log,
	Warning,
	Error
};

struct DebugLogEntry
{
	unsigned long long sequence = 0;
	DebugLogLevel level = DebugLogLevel::Log;
	std::string message;
};

class ENGINEFRAMEWORK_API DebugLog
{
public:
	static void Log(const std::string& message);
	static void Warning(const std::string& message);
	static void Error(const std::string& message);
	static void Clear();
	static const std::vector<DebugLogEntry>& GetEntries();

private:
	static void AddEntry(DebugLogLevel level, const std::string& message);
};
