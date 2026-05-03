#include "pch.h"
#include "EditorDebugLogWindow.h"

#include "../DebugLog.h"

namespace
{
	const char* GetLevelLabel(DebugLogLevel level)
	{
		switch (level)
		{
		case DebugLogLevel::Warning:
			return "Warning";
		case DebugLogLevel::Error:
			return "Error";
		case DebugLogLevel::Log:
		default:
			return "Log";
		}
	}

	ImVec4 GetLevelColor(DebugLogLevel level)
	{
		switch (level)
		{
		case DebugLogLevel::Warning:
			return ImVec4(1.0f, 0.82f, 0.25f, 1.0f);
		case DebugLogLevel::Error:
			return ImVec4(1.0f, 0.35f, 0.35f, 1.0f);
		case DebugLogLevel::Log:
		default:
			return ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
		}
	}
}

void EditorDebugLogWindow::Draw()
{
	static size_t previousEntryCount = 0;

	if (!ImGui::Begin("Debug Log"))
	{
		ImGui::End();
		return;
	}

	if (ImGui::Button("Clear"))
	{
		DebugLog::Clear();
		previousEntryCount = 0;
	}

	ImGui::Separator();
	ImGui::BeginChild("DebugLogEntries", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

	const std::vector<DebugLogEntry>& entries = DebugLog::GetEntries();
	for (std::vector<DebugLogEntry>::const_iterator itr = entries.begin(); itr != entries.end(); ++itr)
	{
		const ImVec4 color = GetLevelColor(itr->level);
		ImGui::PushStyleColor(ImGuiCol_Text, color);
		ImGui::Text("[%llu] [%s] %s", itr->sequence, GetLevelLabel(itr->level), itr->message.c_str());
		ImGui::PopStyleColor();
	}

	if (entries.size() > previousEntryCount)
	{
		ImGui::SetScrollHereY(1.0f);
	}
	previousEntryCount = entries.size();

	ImGui::EndChild();
	ImGui::End();
}
