#pragma once

struct BuildSettingsData
{
	int version = 1;
	std::string startScene;
	std::vector<std::string> scenes;
};

class BuildSettingsManager
{
public:
	static bool Load(BuildSettingsData& outSettings);
	static BuildSettingsData CreateDefault(const std::vector<std::string>& availableScenes);
	static bool Validate(BuildSettingsData& settings);
	static bool LoadOrCreateDefault(BuildSettingsData& outSettings, const std::vector<std::string>& availableScenes);
	static std::string GetBuildSettingsPath();
	static bool Save(BuildSettingsData settings);
	static bool SaveToPath(BuildSettingsData settings, const std::string& path);
	static bool TryGetStartupSceneName(std::string& outSceneName);
};
