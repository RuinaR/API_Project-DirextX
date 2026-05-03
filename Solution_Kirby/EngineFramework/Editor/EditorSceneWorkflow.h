#pragma once

#include <string>

class EditorSceneWorkflow
{
public:
	// EditorSceneWorkflow는 editor 전용 scene workflow를 담당한다.
	// snapshot/dirty 기준선 관리와 New/Open/Save As workflow,
	// rollback 시 dirty/current scene name 복원 정책을 포함한다.
	// JSON 직렬화 코어는 직접 담당하지 않고
	// SceneSerializationService와 SceneDataManager facade를 이용한다.
	static bool CaptureSceneSnapshot(const std::string& sceneName);
	static std::string GetCapturedSnapshot(const std::string& sceneName);
	static void MarkSceneDirty(const std::string& sceneName);
	static void ClearSceneDirty(const std::string& sceneName);
	static bool IsSceneDirty(const std::string& sceneName);
	static bool SaveCapturedSnapshot(const std::string& sceneName);
	static bool CreateNewScene(const std::string& sceneName);
	static bool SaveSceneDataAs(const std::string& sceneName);
	static bool OpenSceneData(const std::string& sceneName);
	static void UpdateCapturedSnapshot(const std::string& sceneName, const std::string& snapshot);
};
