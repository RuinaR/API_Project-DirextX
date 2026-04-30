#include "pch.h"
#include "SceneDataManager.h"
#include "ObjectManager.h"
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

	std::map<std::string, std::string>& GetCapturedSceneSnapshots()
	{
		static std::map<std::string, std::string> snapshots;
		return snapshots;
	}

	std::map<std::string, bool>& GetSceneDirtyFlags()
	{
		static std::map<std::string, bool> dirtyFlags;
		return dirtyFlags;
	}

	std::string BuildSceneDataJson(const std::string& sceneName)
	{
		constexpr int kSceneDataVersion = 4;
		const D3DXVECTOR3 cameraPosition = Camera::GetInstance()->GetPos();
		const D3DXVECTOR3 cameraRotation = Camera::GetInstance()->GetRotation();
		const CameraProjectionMode projectionMode = Camera::GetInstance()->GetProjectionMode();
		const float fov = Camera::GetInstance()->GetFov();
		const float orthographicSize = Camera::GetInstance()->GetOrthographicSize();
		const float nearClip = Camera::GetInstance()->GetNearClip();
		const float farClip = Camera::GetInstance()->GetFarClip();

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"version\": " << kSceneDataVersion << ",\n";
		oss << "  \"sceneName\": \"" << sceneName << "\",\n";
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
		oss << ObjectManager::GetInstance()->SerializeObjects(kSceneDataVersion);
		oss << "\n}\n";
		return oss.str();
	}

	void DeserializeCamera(const std::string& sceneJson)
	{
		std::string cameraJson;
		if (!SceneJson::ExtractObject(sceneJson, "camera", cameraJson))
		{
			return;
		}

		Camera::GetInstance()->InitializeView();
		D3DXVECTOR3 position = Camera::GetInstance()->GetPos();
		D3DXVECTOR3 rotation = Camera::GetInstance()->GetRotation();
		SceneJson::ReadVector3(cameraJson, "position", &position);
		SceneJson::ReadVector3(cameraJson, "rotation", &rotation);

		Camera::GetInstance()->SetPos(position.x, position.y, position.z);
		Camera::GetInstance()->SetRotation(&rotation);

		std::string projectionJson;
		if (!SceneJson::ExtractObject(cameraJson, "projection", projectionJson))
		{
			return;
		}

		std::string projectionMode;
		if (SceneJson::ReadString(projectionJson, "mode", projectionMode))
		{
			Camera::GetInstance()->SetProjectionMode(ParseProjectionModeString(projectionMode));
		}

		float fov = Camera::GetInstance()->GetFov();
		if (SceneJson::ReadFloat(projectionJson, "fov", fov))
		{
			Camera::GetInstance()->SetFov(fov);
		}

		float orthographicSize = Camera::GetInstance()->GetOrthographicSize();
		if (SceneJson::ReadFloat(projectionJson, "orthographicSize", orthographicSize))
		{
			Camera::GetInstance()->SetOrthographicSize(orthographicSize);
		}

		float nearClip = Camera::GetInstance()->GetNearClip();
		if (SceneJson::ReadFloat(projectionJson, "nearClip", nearClip))
		{
			Camera::GetInstance()->SetNearClip(nearClip);
		}

		float farClip = Camera::GetInstance()->GetFarClip();
		if (SceneJson::ReadFloat(projectionJson, "farClip", farClip))
		{
			Camera::GetInstance()->SetFarClip(farClip);
		}
	}

	bool WriteSceneDataFile(const std::string& sceneName, const std::string& json)
	{
		const std::string path = SceneDataManager::GetSceneDataPath(sceneName);
		std::ofstream file(path.c_str(), std::ios::out | std::ios::trunc);
		if (!file.is_open())
		{
			std::cout << "SceneData save failed: " << path << std::endl;
			return false;
		}

		file << json;
		std::cout << "SceneData saved: " << path << std::endl;
		return true;
	}

	bool ReadSceneDataFile(const std::string& sceneName, std::string* outSceneJson)
	{
		if (outSceneJson == nullptr)
		{
			return false;
		}

		const std::string path = SceneDataManager::GetSceneDataPath(sceneName);
		std::ifstream file(path.c_str(), std::ios::in);
		if (!file.is_open())
		{
			std::cout << "SceneData load failed: " << path << std::endl;
			return false;
		}

		std::ostringstream oss;
		oss << file.rdbuf();
		*outSceneJson = oss.str();
		return true;
	}

	bool DeserializeSceneDataJson(const std::string& sceneName, const std::string& sceneJson)
	{
		int sceneVersion = 3;
		SceneJson::ReadInt(sceneJson, "version", sceneVersion);
		DeserializeCamera(sceneJson);
		if (!ObjectManager::GetInstance()->DeserializeObjects(sceneJson, sceneVersion))
		{
			std::cout << "SceneData load failed: " << SceneDataManager::GetSceneDataPath(sceneName) << std::endl;
			return false;
		}

		ObjectManager::GetInstance()->FlushPendingObjects();
		SceneDataManager::CaptureSceneSnapshot(sceneName);
		return true;
	}

	std::string TrimSceneFileExtension(const std::string& fileName)
	{
		const size_t extensionPos = fileName.rfind(".json");
		if (extensionPos == std::string::npos)
		{
			return fileName;
		}
		return fileName.substr(0, extensionPos);
	}

	bool ContainsOnlyWhitespace(const std::string& text)
	{
		for (std::string::const_iterator itr = text.begin(); itr != text.end(); ++itr)
		{
			if (!isspace(static_cast<unsigned char>(*itr)))
			{
				return false;
			}
		}
		return true;
	}
}

std::string SceneDataManager::GetExeDirectory()
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

std::string SceneDataManager::GetFullPath(const std::string& path)
{
	char fullPath[MAX_PATH] = { 0 };
	if (GetFullPathNameA(path.c_str(), MAX_PATH, fullPath, nullptr) == 0)
	{
		return path;
	}
	return fullPath;
}

bool SceneDataManager::DirectoryExists(const std::string& path)
{
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool SceneDataManager::EnsureDirectory(const std::string& path)
{
	if (DirectoryExists(path))
	{
		return true;
	}

	return CreateDirectoryA(path.c_str(), nullptr) == TRUE || GetLastError() == ERROR_ALREADY_EXISTS;
}

std::string SceneDataManager::JoinPath(const std::string& lhs, const std::string& rhs)
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

std::string SceneDataManager::GetSceneDataDirectory()
{
	std::string exeDir = GetExeDirectory();
	std::string sceneDataFolder = "SceneData";
	std::string exeCandidate = JoinPath(exeDir, sceneDataFolder);
	if (DirectoryExists(exeCandidate))
	{
		return exeCandidate;
	}

	std::string relativeSceneData = "..\\..\\SceneData";
	std::string solutionPath = JoinPath(exeDir, relativeSceneData);
	std::string solutionCandidate = GetFullPath(solutionPath);
	if (DirectoryExists(solutionCandidate))
	{
		return solutionCandidate;
	}

	EnsureDirectory(exeCandidate);
	return exeCandidate;
}

std::string SceneDataManager::GetSceneDataPath(const std::string& sceneName)
{
	std::string sceneDataDirectory = GetSceneDataDirectory();
	std::string fileName = sceneName + ".json";
	return JoinPath(sceneDataDirectory, fileName);
}

bool SceneDataManager::Exists(const std::string& sceneName)
{
	const std::string path = GetSceneDataPath(sceneName);
	DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY);
}

bool SceneDataManager::IsValidSceneName(const std::string& sceneName)
{
	if (sceneName.empty() || ContainsOnlyWhitespace(sceneName))
	{
		return false;
	}

	static const char* kInvalidSceneNameChars = "\\/:*?\"<>|";
	return sceneName.find_first_of(kInvalidSceneNameChars) == std::string::npos;
}

bool SceneDataManager::SaveInitialSceneData(const std::string& sceneName)
{
	return SaveCurrentSceneData(sceneName);
}

bool SceneDataManager::SaveSceneData(const std::string& sceneName)
{
	return SaveCurrentSceneData(sceneName);
}

bool SceneDataManager::SaveCurrentSceneData(const std::string& sceneName)
{
	const std::string json = BuildSceneDataJson(sceneName);
	if (!WriteSceneDataFile(sceneName, json))
	{
		return false;
	}

	GetCapturedSceneSnapshots()[sceneName] = json;
	ClearSceneDirty(sceneName);
	return true;
}

bool SceneDataManager::CaptureSceneSnapshot(const std::string& sceneName)
{
	GetCapturedSceneSnapshots()[sceneName] = BuildSceneDataJson(sceneName);
	ClearSceneDirty(sceneName);
	std::cout << "SceneData snapshot captured: " << sceneName << std::endl;
	return true;
}

std::string SceneDataManager::GetCapturedSnapshot(const std::string& sceneName)
{
	std::map<std::string, std::string>::iterator itr = GetCapturedSceneSnapshots().find(sceneName);
	if (itr == GetCapturedSceneSnapshots().end())
	{
		return std::string();
	}
	return itr->second;
}

void SceneDataManager::MarkSceneDirty(const std::string& sceneName)
{
	if (sceneName.empty())
	{
		return;
	}
	GetSceneDirtyFlags()[sceneName] = true;
}

void SceneDataManager::ClearSceneDirty(const std::string& sceneName)
{
	if (sceneName.empty())
	{
		return;
	}
	GetSceneDirtyFlags()[sceneName] = false;
}

bool SceneDataManager::IsSceneDirty(const std::string& sceneName)
{
	std::map<std::string, bool>::iterator dirtyItr = GetSceneDirtyFlags().find(sceneName);
	if (dirtyItr != GetSceneDirtyFlags().end() && dirtyItr->second)
	{
		return true;
	}

	const std::string snapshot = GetCapturedSnapshot(sceneName);
	if (snapshot.empty())
	{
		return false;
	}
	return snapshot != BuildSceneDataJson(sceneName);
}

bool SceneDataManager::SaveCapturedSnapshot(const std::string& sceneName)
{
	const std::string snapshot = GetCapturedSnapshot(sceneName);
	if (snapshot.empty())
	{
		std::cout << "SceneData snapshot not found: " << sceneName << std::endl;
		return false;
	}
	const bool saved = WriteSceneDataFile(sceneName, snapshot);
	if (saved)
	{
		ClearSceneDirty(sceneName);
	}
	return saved;
}

bool SceneDataManager::LoadSceneData(const std::string& sceneName)
{
	std::string sceneJson;
	if (!ReadSceneDataFile(sceneName, &sceneJson))
	{
		return false;
	}

	return DeserializeSceneDataJson(sceneName, sceneJson);
}

bool SceneDataManager::CreateNewScene(const std::string& sceneName)
{
	if (sceneName.empty() || WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return false;
	}

	objectManager->Clear();
	objectManager->FlushPendingObjects();
	Camera::GetInstance()->InitializeView();
	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	MarkSceneDirty(sceneName);
	return true;
}

std::vector<std::string> SceneDataManager::GetSceneFileList()
{
	std::vector<std::string> sceneNames;
	const std::string searchPattern = JoinPath(GetSceneDataDirectory(), "*.json");

	WIN32_FIND_DATAA findData = {};
	HANDLE findHandle = FindFirstFileA(searchPattern.c_str(), &findData);
	if (findHandle == INVALID_HANDLE_VALUE)
	{
		return sceneNames;
	}

	do
	{
		if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
		{
			continue;
		}

		sceneNames.push_back(TrimSceneFileExtension(findData.cFileName));
	} while (FindNextFileA(findHandle, &findData) == TRUE);

	FindClose(findHandle);
	std::sort(sceneNames.begin(), sceneNames.end());
	return sceneNames;
}

bool SceneDataManager::OpenSceneData(const std::string& sceneName)
{
	if (sceneName.empty() || WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager == nullptr)
	{
		return false;
	}

	std::string targetSceneJson;
	if (!ReadSceneDataFile(sceneName, &targetSceneJson))
	{
		return false;
	}

	const char* currentSceneName = WindowFrame::GetInstance()->GetCurrentSceneName();
	const std::string previousSceneName = currentSceneName != nullptr ? currentSceneName : "";
	const bool previousSceneDirty = !previousSceneName.empty() && IsSceneDirty(previousSceneName);
	const std::string previousSceneJson = previousSceneName.empty() ? std::string() : BuildSceneDataJson(previousSceneName);

	// TODO: dirty 상태 저장 확인 팝업은 Open Scene / Save As 흐름과 함께 다음 단계에서 정리한다.
	objectManager->Clear();
	objectManager->FlushPendingObjects();

	if (!DeserializeSceneDataJson(sceneName, targetSceneJson))
	{
		objectManager->Clear();
		objectManager->FlushPendingObjects();

		if (!previousSceneJson.empty() && DeserializeSceneDataJson(previousSceneName, previousSceneJson))
		{
			WindowFrame::GetInstance()->SetCurrentSceneName(previousSceneName);
			if (previousSceneDirty)
			{
				MarkSceneDirty(previousSceneName);
			}
			else
			{
				ClearSceneDirty(previousSceneName);
			}
		}
		return false;
	}

	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	ClearSceneDirty(sceneName);
	return true;
}

bool SceneDataManager::SaveSceneDataAs(const std::string& sceneName)
{
	if (WindowFrame::GetInstance() == nullptr || WindowFrame::GetInstance()->GetRenderType() != RenderType::Edit)
	{
		return false;
	}

	if (!IsValidSceneName(sceneName))
	{
		std::cout << "Save Scene As failed: invalid scene name: " << sceneName << std::endl;
		return false;
	}

	if (!SaveCurrentSceneData(sceneName))
	{
		return false;
	}

	WindowFrame::GetInstance()->SetCurrentSceneName(sceneName);
	ClearSceneDirty(sceneName);
	return true;
}
