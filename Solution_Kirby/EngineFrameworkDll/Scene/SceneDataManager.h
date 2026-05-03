#pragma once

#include "../EngineFrameworkAPI.h"
#include <string>
#include <vector>

class ENGINEFRAMEWORK_API SceneDataManager
{
private:
	static std::string GetExeDirectory();
	static std::string GetFullPath(const std::string& path);
	static bool DirectoryExists(const std::string& path);
	static bool EnsureDirectory(const std::string& path);
	static std::string JoinPath(const std::string& lhs, const std::string& rhs);

public:
	// Runtime-facing facade for scene path/file list/name validation,
	// save/load entry points, and loading scene version access.
	// Serialization core lives in SceneSerializationService, while
	// editor scene workflow lives in EditorSceneWorkflow.
	static std::string GetSceneDataDirectory();
	static std::string GetSceneDataPath(const std::string& sceneName);
	static bool Exists(const std::string& sceneName);
	static bool SaveInitialSceneData(const std::string& sceneName);
	static bool SaveSceneData(const std::string& sceneName);
	static bool SaveCurrentSceneData(const std::string& sceneName);
	static bool LoadSceneData(const std::string& sceneName);

	// Workflow-facing deserialize entry point.
	// This keeps loading scene version storage and failure logging in the facade,
	// while delegating actual JSON deserialize core to SceneSerializationService.
	static bool DeserializeSceneDataForWorkflow(const std::string& sceneName, const std::string& sceneJson);

	static std::vector<std::string> GetSceneFileList();
	static bool IsValidSceneName(const std::string& sceneName);
	static int GetCurrentLoadingSceneVersion();
};
