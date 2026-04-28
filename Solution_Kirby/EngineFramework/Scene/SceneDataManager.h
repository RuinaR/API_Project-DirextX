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
	// 기존 SceneData 재저장 절차:
	// 1. SceneData/*.json 파일을 백업한다.
	// 2. BuildEdit을 실행한 뒤 대상 씬을 연다.
	// 3. Hierarchy 창의 Save Scene 버튼으로 현재 씬을 version 3 형식으로 다시 저장한다.
	// 자동 마이그레이션은 로드 실패 시 대체 흐름을 단순하게 유지하기 위해 보류한다.
	static std::string GetSceneDataDirectory();
	static std::string GetSceneDataPath(const std::string& sceneName);
	static bool Exists(const std::string& sceneName);
	static bool SaveInitialSceneData(const std::string& sceneName);
	static bool SaveSceneData(const std::string& sceneName);
	static bool SaveCurrentSceneData(const std::string& sceneName);
	static bool CaptureSceneSnapshot(const std::string& sceneName);
	static std::string GetCapturedSnapshot(const std::string& sceneName);
	static bool IsSceneDirty(const std::string& sceneName);
	static bool SaveCapturedSnapshot(const std::string& sceneName);
	static bool LoadSceneData(const std::string& sceneName);
};
