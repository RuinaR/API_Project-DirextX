#pragma once

#include <string>

class MainFrame;
class Camera;
class ObjectManager;

class SceneSerializationService
{
public:
	// SceneSerializationService는 순수 scene serialization core를 담당한다.
	// scene file read/write, scene json build, scene json deserialize core,
	// camera/timeScale deserialize를 제공한다.
	// editor dirty/snapshot/open workflow는 여기서 담당하지 않는다.
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
