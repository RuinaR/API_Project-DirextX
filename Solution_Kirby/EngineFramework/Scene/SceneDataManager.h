#pragma once

class SceneDataManager
{
private:
	static std::string GetExeDirectory();
	static std::string GetFullPath(const std::string& path);
	static bool DirectoryExists(const std::string& path);
	static bool EnsureDirectory(const std::string& path);
	static std::string JoinPath(const std::string& lhs, const std::string& rhs);

public:
	// SceneDataManager는 현재 scene path/file list/name validation,
	// runtime save/load 진입점, loading scene version 조회를 제공하는 facade다.
	// 실제 직렬화 코어는 SceneSerializationService,
	// editor snapshot/dirty/open/new/save-as workflow는 EditorSceneWorkflow로 이동했다.
	static std::string GetSceneDataDirectory();
	static std::string GetSceneDataPath(const std::string& sceneName);
	static bool Exists(const std::string& sceneName);
	static bool SaveInitialSceneData(const std::string& sceneName);
	static bool SaveSceneData(const std::string& sceneName);
	static bool SaveCurrentSceneData(const std::string& sceneName);
	static bool CaptureSceneSnapshot(const std::string& sceneName);
	static std::string GetCapturedSnapshot(const std::string& sceneName);
	static void MarkSceneDirty(const std::string& sceneName);
	static void ClearSceneDirty(const std::string& sceneName);
	static bool IsSceneDirty(const std::string& sceneName);
	static bool SaveCapturedSnapshot(const std::string& sceneName);
	static bool LoadSceneData(const std::string& sceneName);
	// Open Scene / rollback 같은 editor workflow가 scene json을 직접 역직렬화할 때 쓰는 진입점이다.
	// loading scene version 저장소와 실패 로그는 여기서 관리하고, snapshot은 여기서 찍지 않는다.
	static bool DeserializeSceneDataForWorkflow(const std::string& sceneName, const std::string& sceneJson);
	// 아래 3개 함수는 현재 EditorSceneWorkflow로 위임되는 wrapper다.
	static bool CreateNewScene(const std::string& sceneName = "NewScene");
	static std::vector<std::string> GetSceneFileList();
	static bool OpenSceneData(const std::string& sceneName);
	static bool IsValidSceneName(const std::string& sceneName);
	static bool SaveSceneDataAs(const std::string& sceneName);
	static int GetCurrentLoadingSceneVersion();
};
