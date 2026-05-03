#include "pch.h"
#include "DebugLog.h"

namespace
{
	std::vector<DebugLogEntry>& GetDebugLogStorage()
	{
		static std::vector<DebugLogEntry> entries;
		return entries;
	}

	unsigned long long& GetDebugLogSequence()
	{
		static unsigned long long sequence = 0;
		return sequence;
	}
}

void DebugLog::Log(const std::string& message)
{
	AddEntry(DebugLogLevel::Log, message);
}

void DebugLog::Warning(const std::string& message)
{
	AddEntry(DebugLogLevel::Warning, message);
}

void DebugLog::Error(const std::string& message)
{
	AddEntry(DebugLogLevel::Error, message);
}

void DebugLog::Clear()
{
	GetDebugLogStorage().clear();
}

const std::vector<DebugLogEntry>& DebugLog::GetEntries()
{
	return GetDebugLogStorage();
}

void DebugLog::AddEntry(DebugLogLevel level, const std::string& message)
{
	DebugLogEntry entry;
	entry.sequence = ++GetDebugLogSequence();
	entry.level = level;
	entry.message = message;
	GetDebugLogStorage().push_back(entry);
}
