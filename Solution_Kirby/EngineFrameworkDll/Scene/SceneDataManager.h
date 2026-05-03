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
	// 이 클래스는 SceneData 경로, 파일 목록, 이름 검사, 저장/로드 진입점을 맡는다.
	// 실제 JSON 저장/복원 코어는 SceneSerializationService가 맡고,
	// 에디터 쪽 scene 작업 흐름은 EditorSceneWorkflow가 맡는다.
	static std::string GetSceneDataDirectory();
	static std::string GetSceneDataPath(const std::string& sceneName);
	static bool Exists(const std::string& sceneName);
	static bool SaveInitialSceneData(const std::string& sceneName);
	static bool SaveSceneData(const std::string& sceneName);
	static bool SaveCurrentSceneData(const std::string& sceneName);
	static bool LoadSceneData(const std::string& sceneName);

	// 에디터 작업 흐름에서 쓰는 scene 복원 진입점이다.
	// 여기서는 로드 중 버전 보관과 실패 로그를 맡고,
	// 실제 JSON 복원 코어는 SceneSerializationService에 맡긴다.
	static bool DeserializeSceneDataForWorkflow(const std::string& sceneName, const std::string& sceneJson);

	static std::vector<std::string> GetSceneFileList();
	static bool IsValidSceneName(const std::string& sceneName);
	static int GetCurrentLoadingSceneVersion();
};
