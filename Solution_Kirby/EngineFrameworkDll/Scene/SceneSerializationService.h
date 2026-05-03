#pragma once

#include "../EngineFrameworkAPI.h"
#include <string>

class MainFrame;
class Camera;
class ObjectManager;

class ENGINEFRAMEWORK_API SceneSerializationService
{
public:
	// SceneSerializationService는 scene 저장/로드 핵심만 맡는다.
	// scene 파일 읽기/쓰기, scene json 만들기, scene json 복원,
	// camera/timeScale 복원 같은 공통 작업을 여기서 처리한다.
	// editor의 dirty, snapshot, open 같은 작업 흐름은 여기서 맡지 않는다.
	static bool WriteSceneDataFile(const std::string& sceneFilePath, const std::string& json);
	static bool ReadSceneDataFile(const std::string& sceneFilePath, std::string* outSceneJson);
	static void DeserializeTimeScale(const std::string& sceneJson, MainFrame* mainFrame);
	static void DeserializeCamera(const std::string& sceneJson, Camera* camera);
	static std::string BuildSceneDataJson(const std::string& sceneName, MainFrame* mainFrame, Camera* camera, ObjectManager* objectManager);
	static bool DeserializeSceneDataJson(
		const std::string& sceneJson,
		MainFrame* mainFrame,
		Camera* camera,
		ObjectManager* objectManager,
		int* outSceneVersion);
};
