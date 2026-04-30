#include "pch.h"
#include "SceneDataManager.h"
#include "ObjectManager.h"
#include "SceneJsonUtility.h"

namespace
{
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

		std::ostringstream oss;
		oss << "{\n";
		oss << "  \"version\": " << kSceneDataVersion << ",\n";
		oss << "  \"sceneName\": \"" << sceneName << "\",\n";
		oss << "  \"camera\": {\n";
		oss << "    \"position\": { \"x\": " << cameraPosition.x << ", \"y\": " << cameraPosition.y << ", \"z\": " << cameraPosition.z << " },\n";
		oss << "    \"rotation\": { \"x\": " << cameraRotation.x << ", \"y\": " << cameraRotation.y << ", \"z\": " << cameraRotation.z << " }\n";
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

		D3DXVECTOR3 position = Camera::GetInstance()->GetPos();
		D3DXVECTOR3 rotation = Camera::GetInstance()->GetRotation();
		SceneJson::ReadVector3(cameraJson, "position", &position);
		SceneJson::ReadVector3(cameraJson, "rotation", &rotation);

		Camera::GetInstance()->SetPos(position.x, position.y, position.z);
		Camera::GetInstance()->SetRotation(&rotation);
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
	std::string relativeSceneData = "..\\..\\SceneData";
	std::string solutionPath = JoinPath(exeDir, relativeSceneData);
	std::string solutionCandidate = GetFullPath(solutionPath);
	if (EnsureDirectory(solutionCandidate))
	{
		return solutionCandidate;
	}

	std::string sceneDataFolder = "SceneData";
	std::string exeCandidate = JoinPath(exeDir, sceneDataFolder);
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
	const std::string path = GetSceneDataPath(sceneName);
	std::ifstream file(path.c_str(), std::ios::in);
	if (!file.is_open())
	{
		std::cout << "SceneData load failed: " << path << std::endl;
		return false;
	}

	std::ostringstream oss;
	oss << file.rdbuf();
	const std::string sceneJson = oss.str();
	int sceneVersion = 3;
	SceneJson::ReadInt(sceneJson, "version", sceneVersion);
	DeserializeCamera(sceneJson);
	if (!ObjectManager::GetInstance()->DeserializeObjects(sceneJson, sceneVersion))
	{
		std::cout << "SceneData load failed: " << path << std::endl;
		return false;
	}
	CaptureSceneSnapshot(sceneName);
	return true;
}
