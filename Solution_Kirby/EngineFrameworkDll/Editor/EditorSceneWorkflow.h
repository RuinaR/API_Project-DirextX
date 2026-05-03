#pragma once

#include "../EngineFrameworkAPI.h"
#include <string>

class ENGINEFRAMEWORK_API EditorSceneWorkflow
{
public:
	// EditorSceneWorkflow는 에디터에서 쓰는 scene 작업 흐름을 맡는다.
	// snapshot/dirty 기준 관리, New/Open/Save As,
	// 실패했을 때 이전 scene으로 되돌리는 흐름도 여기서 다룬다.
	// JSON 저장/복원 핵심은 직접 처리하지 않고
	// SceneSerializationService와 SceneDataManager를 이용한다.
	static bool CaptureSceneSnapshot(const std::string& sceneName);
	static std::string GetCapturedSnapshot(const std::string& sceneName);
	static void MarkSceneDirty(const std::string& sceneName);
	static void ClearSceneDirty(const std::string& sceneName);
	static bool IsSceneDirty(const std::string& sceneName);
	static bool SaveCapturedSnapshot(const std::string& sceneName);
	static bool SaveCurrentSceneDataWithBaseline(const std::string& sceneName);
	static bool CreateNewScene(const std::string& sceneName = "NewScene");
	static bool SaveSceneDataAs(const std::string& sceneName);
	static bool OpenSceneData(const std::string& sceneName);
	static void UpdateCapturedSnapshot(const std::string& sceneName, const std::string& snapshot);
};
