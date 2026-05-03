#pragma once

#include <string>

namespace SceneChangeAction
{
	static const char* const kStartToGameActionKey = "SceneChange.StartToGame";
	static const char* const kResultToStartActionKey = "SceneChange.ResultToStart";

	void RegisterActions();
	void RequestSceneChange(const std::string& targetSceneName);
}
