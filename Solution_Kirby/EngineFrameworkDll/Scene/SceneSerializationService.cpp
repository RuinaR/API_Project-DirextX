#include "pch.h"
#include "SceneSerializationService.h"
#include "MainFrame.h"
#include "ObjectManager.h"
#include "Rendering/Camera/Camera.h"
#include "SceneJsonUtility.h"

namespace
{
	const char* ToProjectionModeString(CameraProjectionMode mode)
	{
		return mode == CameraProjectionMode::Perspective ? "Perspective" : "Orthographic";
	}

	CameraProjectionMode ParseProjectionModeString(const std::string& mode)
	{
		if (mode == "Perspective")
		{
			return CameraProjectionMode::Perspective;
		}

		return CameraProjectionMode::Orthographic;
	}
}

std::string SceneSerializationService::BuildSceneDataJson(const std::string& sceneName, MainFrame* mainFrame, Camera* camera, ObjectManager* objectManager)
{
	constexpr int kSceneDataVersion = 9;
	const D3DXVECTOR3 cameraPosition = camera != nullptr ? camera->GetPos() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	const D3DXVECTOR3 cameraRotation = camera != nullptr ? camera->GetRotation() : D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	const CameraProjectionMode projectionMode = camera != nullptr ? camera->GetProjectionMode() : CameraProjectionMode::Orthographic;
	const float fov = camera != nullptr ? camera->GetFov() : 0.0f;
	const float orthographicSize = camera != nullptr ? camera->GetOrthographicSize() : 0.0f;
	const float nearClip = camera != nullptr ? camera->GetNearClip() : 0.0f;
	const float farClip = camera != nullptr ? camera->GetFarClip() : 0.0f;

	std::ostringstream oss;
	oss << "{\n";
	oss << "  \"version\": " << kSceneDataVersion << ",\n";
	oss << "  \"sceneName\": \"" << sceneName << "\",\n";
	oss << "  \"timeScale\": " << (mainFrame != nullptr ? mainFrame->GetTimeScale() : 1.0f) << ",\n";
	oss << "  \"camera\": {\n";
	oss << "    \"position\": { \"x\": " << cameraPosition.x << ", \"y\": " << cameraPosition.y << ", \"z\": " << cameraPosition.z << " },\n";
	oss << "    \"rotation\": { \"x\": " << cameraRotation.x << ", \"y\": " << cameraRotation.y << ", \"z\": " << cameraRotation.z << " },\n";
	oss << "    \"projection\": {\n";
	oss << "      \"mode\": \"" << ToProjectionModeString(projectionMode) << "\",\n";
	oss << "      \"fov\": " << fov << ",\n";
	oss << "      \"orthographicSize\": " << orthographicSize << ",\n";
	oss << "      \"nearClip\": " << nearClip << ",\n";
	oss << "      \"farClip\": " << farClip << "\n";
	oss << "    }\n";
	oss << "  },\n";
	oss << "  \"objects\": ";
	oss << (objectManager != nullptr ? objectManager->SerializeObjects(kSceneDataVersion) : "[]");
	oss << "\n}\n";
	return oss.str();
}

bool SceneSerializationService::WriteSceneDataFile(const std::string& sceneFilePath, const std::string& json)
{
	std::ofstream file(sceneFilePath.c_str(), std::ios::out | std::ios::trunc);
	if (!file.is_open())
	{
		std::cout << "SceneData save failed: " << sceneFilePath << std::endl;
		return false;
	}

	file << json;
	std::cout << "SceneData saved: " << sceneFilePath << std::endl;
	return true;
}

bool SceneSerializationService::ReadSceneDataFile(const std::string& sceneFilePath, std::string* outSceneJson)
{
	if (outSceneJson == nullptr)
	{
		return false;
	}

	std::ifstream file(sceneFilePath.c_str(), std::ios::in);
	if (!file.is_open())
	{
		std::cout << "SceneData load failed: " << sceneFilePath << std::endl;
		return false;
	}

	std::ostringstream oss;
	oss << file.rdbuf();
	*outSceneJson = oss.str();
	return true;
}

void SceneSerializationService::DeserializeTimeScale(const std::string& sceneJson, MainFrame* mainFrame)
{
	if (mainFrame == nullptr)
	{
		return;
	}

	float timeScale = 1.0f;
	if (SceneJson::ReadFloat(sceneJson, "timeScale", timeScale))
	{
		mainFrame->SetTimeScale(timeScale);
		return;
	}

	mainFrame->SetTimeScale(1.0f);
}

void SceneSerializationService::DeserializeCamera(const std::string& sceneJson, Camera* camera)
{
	if (camera == nullptr)
	{
		return;
	}

	std::string cameraJson;
	if (!SceneJson::ExtractObject(sceneJson, "camera", cameraJson))
	{
		return;
	}

	camera->InitializeView();
	D3DXVECTOR3 position = camera->GetPos();
	D3DXVECTOR3 rotation = camera->GetRotation();
	SceneJson::ReadVector3(cameraJson, "position", &position);
	SceneJson::ReadVector3(cameraJson, "rotation", &rotation);

	camera->SetPos(position.x, position.y, position.z);
	camera->SetRotation(&rotation);

	std::string projectionJson;
	if (!SceneJson::ExtractObject(cameraJson, "projection", projectionJson))
	{
		return;
	}

	std::string projectionMode;
	if (SceneJson::ReadString(projectionJson, "mode", projectionMode))
	{
		camera->SetProjectionMode(ParseProjectionModeString(projectionMode));
	}

	float fov = camera->GetFov();
	if (SceneJson::ReadFloat(projectionJson, "fov", fov))
	{
		camera->SetFov(fov);
	}

	float orthographicSize = camera->GetOrthographicSize();
	if (SceneJson::ReadFloat(projectionJson, "orthographicSize", orthographicSize))
	{
		camera->SetOrthographicSize(orthographicSize);
	}

	float nearClip = camera->GetNearClip();
	if (SceneJson::ReadFloat(projectionJson, "nearClip", nearClip))
	{
		camera->SetNearClip(nearClip);
	}

	float farClip = camera->GetFarClip();
	if (SceneJson::ReadFloat(projectionJson, "farClip", farClip))
	{
		camera->SetFarClip(farClip);
	}
}

bool SceneSerializationService::DeserializeSceneDataJson(
	const std::string& sceneJson,
	MainFrame* mainFrame,
	Camera* camera,
	ObjectManager* objectManager,
	int* outSceneVersion)
{
	int sceneVersion = 3;
	SceneJson::ReadInt(sceneJson, "version", sceneVersion);
	if (outSceneVersion != nullptr)
	{
		*outSceneVersion = sceneVersion;
	}

	DeserializeTimeScale(sceneJson, mainFrame);
	DeserializeCamera(sceneJson, camera);

	if (objectManager == nullptr)
	{
		return false;
	}

	if (!objectManager->DeserializeObjects(sceneJson, sceneVersion))
	{
		return false;
	}

	objectManager->FlushPendingObjects();
	return true;
}
