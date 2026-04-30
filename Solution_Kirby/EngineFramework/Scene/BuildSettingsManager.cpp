#include "pch.h"
#include "BuildSettingsManager.h"
#include "SceneJsonUtility.h"

namespace
{
	constexpr int kBuildSettingsVersion = 1;

	std::string GetExeDirectory()
	{
		char path[MAX_PATH] = { 0 };
		GetModuleFileNameA(NULL, path, MAX_PATH);
		std::string exePath = path;
		size_t pos = exePath.find_last_of("\\/");
		if (pos == std::string::npos)
		{
			return std::string(".");
		}
		return exePath.substr(0, pos);
	}

	std::string JoinPath(const std::string& lhs, const std::string& rhs)
	{
		if (lhs.empty())
		{
			return rhs;
		}
		if (lhs.back() == '\\' || lhs.back() == '/')
		{
			return lhs + rhs;
		}
		return lhs + "\\" + rhs;
	}

	std::string GetFullPath(const std::string& path)
	{
		char fullPath[MAX_PATH] = { 0 };
		if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr) == 0)
		{
			return path;
		}
		return fullPath;
	}

	std::string ResolveBuildSettingsPath()
	{
		const std::string exeDir = GetExeDirectory();
		const std::string exeCandidate = JoinPath(exeDir, "BuildSettings.json");
		if (GetFileAttributesA(exeCandidate.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			return exeCandidate;
		}

		const std::string solutionCandidate = GetFullPath(JoinPath(exeDir, "..\\..\\BuildSettings.json"));
		if (GetFileAttributesA(solutionCandidate.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			return solutionCandidate;
		}

		return exeCandidate;
	}

	bool ReadFileText(const std::string& path, std::string& outText)
	{
		std::ifstream file(path.c_str(), std::ios::in);
		if (!file.is_open())
		{
			return false;
		}

		std::ostringstream oss;
		oss << file.rdbuf();
		outText = oss.str();
		return true;
	}

	bool WriteFileText(const std::string& path, const std::string& text)
	{
		std::ofstream file(path.c_str(), std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			return false;
		}

		file << text;
		return true;
	}

	bool ExtractStringsFromArray(const std::string& arrayJson, std::vector<std::string>& outStrings)
	{
		bool inString = false;
		bool escaped = false;
		std::string current;

		for (size_t i = 0; i < arrayJson.length(); ++i)
		{
			const char ch = arrayJson[i];
			if (!inString)
			{
				if (ch == '"')
				{
					inString = true;
					current.clear();
				}
				continue;
			}

			if (escaped)
			{
				switch (ch)
				{
				case 'n':
					current.push_back('\n');
					break;
				case 'r':
					current.push_back('\r');
					break;
				case 't':
					current.push_back('\t');
					break;
				default:
					current.push_back(ch);
					break;
				}
				escaped = false;
				continue;
			}

			if (ch == '\\')
			{
				escaped = true;
				continue;
			}

			if (ch == '"')
			{
				outStrings.push_back(current);
				inString = false;
				continue;
			}

			current.push_back(ch);
		}

		return !inString && !escaped;
	}

	bool ContainsSceneName(const std::vector<std::string>& scenes, const std::string& sceneName)
	{
		for (std::vector<std::string>::const_iterator itr = scenes.begin(); itr != scenes.end(); ++itr)
		{
			if (*itr == sceneName)
			{
				return true;
			}
		}
		return false;
	}

	std::string EscapeJsonString(const std::string& value)
	{
		std::string escaped;
		for (std::string::const_iterator itr = value.begin(); itr != value.end(); ++itr)
		{
			switch (*itr)
			{
			case '\\':
				escaped += "\\\\";
				break;
			case '"':
				escaped += "\\\"";
				break;
			case '\n':
				escaped += "\\n";
				break;
			case '\r':
				escaped += "\\r";
				break;
			case '\t':
				escaped += "\\t";
				break;
			default:
				escaped.push_back(*itr);
				break;
			}
		}
		return escaped;
	}

	std::string BuildSettingsJson(const BuildSettingsData& settings)
	{
		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"version\": " << settings.version << ",\n";
		oss << "  \"startScene\": \"" << EscapeJsonString(settings.startScene) << "\",\n";
		oss << "  \"scenes\": [";
		for (size_t i = 0; i < settings.scenes.size(); ++i)
		{
			if (i > 0)
			{
				oss << ", ";
			}
			oss << "\"" << EscapeJsonString(settings.scenes[i]) << "\"";
		}
		oss << "]\n";
		oss << "}\n";
		return oss.str();
	}
}

bool BuildSettingsManager::Load(BuildSettingsData& outSettings)
{
	outSettings = BuildSettingsData();

	std::string settingsJson;
	if (!ReadFileText(GetBuildSettingsPath(), settingsJson))
	{
		return false;
	}

	SceneJson::ReadInt(settingsJson, "version", outSettings.version);
	if (!SceneJson::ReadString(settingsJson, "startScene", outSettings.startScene))
	{
		return false;
	}

	std::string scenesArrayJson;
	if (!SceneJson::ExtractArray(settingsJson, "scenes", scenesArrayJson))
	{
		return false;
	}

	if (!ExtractStringsFromArray(scenesArrayJson, outSettings.scenes))
	{
		return false;
	}

	if (outSettings.startScene.empty() || !ContainsSceneName(outSettings.scenes, outSettings.startScene))
	{
		return false;
	}

	return true;
}

BuildSettingsData BuildSettingsManager::CreateDefault(const std::vector<std::string>& availableScenes)
{
	BuildSettingsData settings;
	settings.version = kBuildSettingsVersion;
	settings.scenes = availableScenes;

	if (ContainsSceneName(availableScenes, "StartScene"))
	{
		settings.startScene = "StartScene";
	}
	else if (!availableScenes.empty())
	{
		settings.startScene = availableScenes.front();
	}

	Validate(settings);
	return settings;
}

bool BuildSettingsManager::Validate(BuildSettingsData& settings)
{
	settings.version = kBuildSettingsVersion;

	std::vector<std::string> uniqueScenes;
	for (std::vector<std::string>::const_iterator itr = settings.scenes.begin(); itr != settings.scenes.end(); ++itr)
	{
		if (itr->empty() || ContainsSceneName(uniqueScenes, *itr))
		{
			continue;
		}
		uniqueScenes.push_back(*itr);
	}
	settings.scenes.swap(uniqueScenes);

	if (settings.scenes.empty())
	{
		settings.startScene.clear();
		return false;
	}

	if (!ContainsSceneName(settings.scenes, settings.startScene))
	{
		if (ContainsSceneName(settings.scenes, "StartScene"))
		{
			settings.startScene = "StartScene";
		}
		else
		{
			settings.startScene = settings.scenes.front();
		}
	}

	return !settings.startScene.empty() && ContainsSceneName(settings.scenes, settings.startScene);
}

bool BuildSettingsManager::LoadOrCreateDefault(BuildSettingsData& outSettings, const std::vector<std::string>& availableScenes)
{
	if (!Load(outSettings))
	{
		outSettings = CreateDefault(availableScenes);
		return Validate(outSettings);
	}

	std::vector<std::string> filteredScenes;
	for (std::vector<std::string>::const_iterator itr = outSettings.scenes.begin(); itr != outSettings.scenes.end(); ++itr)
	{
		if (!ContainsSceneName(availableScenes, *itr))
		{
			continue;
		}
		filteredScenes.push_back(*itr);
	}
	outSettings.scenes.swap(filteredScenes);

	return Validate(outSettings);
}

std::string BuildSettingsManager::GetBuildSettingsPath()
{
	return ResolveBuildSettingsPath();
}

bool BuildSettingsManager::Save(BuildSettingsData settings)
{
	return SaveToPath(settings, GetBuildSettingsPath());
}

bool BuildSettingsManager::SaveToPath(BuildSettingsData settings, const std::string& path)
{
	if (!Validate(settings) || path.empty())
	{
		return false;
	}

	return WriteFileText(path, BuildSettingsJson(settings));
}

bool BuildSettingsManager::TryGetStartupSceneName(std::string& outSceneName)
{
	BuildSettingsData settings;
	if (!Load(settings))
	{
		outSceneName.clear();
		return false;
	}

	outSceneName = settings.startScene;
	return !outSceneName.empty();
}
